#include "plugin.hpp"

#include <filesystem>

#include "sql_resource.h"
#include "tool.hpp"

#include <SQLiteCpp/SQLiteCpp.h>
#include <ankerl/unordered_dense.h>
#include <reframework/API.hpp>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>

using namespace reframework;

struct FieldIdMap {
    ankerl::unordered_dense::segmented_map<API::Field *, int32_t> Static;
    ankerl::unordered_dense::segmented_map<API::Field *, int32_t> Instance;
};

struct MethodIdMap {
    ankerl::unordered_dense::segmented_map<API::Method *, int32_t> Static;
    ankerl::unordered_dense::segmented_map<API::Method *, int32_t> Instance;
};

struct TDBIdMap {
    ankerl::unordered_dense::segmented_map<API::TypeDefinition *, int32_t> Type;
    FieldIdMap Field;
    MethodIdMap Method;

    static TDBIdMap get() {
        TDBIdMap result;
        const auto &api = API::get();
        const auto tdb = api->tdb();

        for (auto i = 0; i < tdb->get_num_types(); ++i) {
            const auto type = tdb->get_type(i);
            if (type == nullptr) {
                continue;
            }
            result.Type.insert({type, i});
        }

        for (auto i = 0; i < tdb->get_num_fields(); ++i) {
            const auto field = tdb->get_field(i);
            if (field == nullptr) {
                continue;
            }
            if (field->is_static()) {
                result.Field.Static.insert({field, i});
            } else {
                result.Field.Instance.insert({field, i});
            }
        }

        for (auto i = 0; i < tdb->get_num_methods(); ++i) {
            const auto method = tdb->get_method(i);
            if (method == nullptr) {
                continue;
            }
            if (method->is_static()) {
                result.Method.Static.insert({method, i});
            } else {
                result.Method.Instance.insert({method, i});
            }
        }

        return result;
    }
};

extern "C" __declspec(dllexport) void reframework_plugin_required_version(REFrameworkPluginVersion *version) {
    version->major = REFRAMEWORK_PLUGIN_VERSION_MAJOR;
    version->minor = REFRAMEWORK_PLUGIN_VERSION_MINOR;
    version->patch = REFRAMEWORK_PLUGIN_VERSION_PATCH;
}

extern "C" __declspec(dllexport) bool reframework_plugin_initialize(const REFrameworkPluginInitializeParam *param) {
    API::initialize(param);
    const std::filesystem::path path = std::filesystem::current_path();
    const std::string targetDirName = "reframework";
    if (const auto foundPath = Tool::findDirectoryIgnoreCase(path, targetDirName)) {
        constexpr auto name = "[BPQS]";
        const auto logPath = *foundPath / "log.txt";
        auto logger = spdlog::basic_logger_mt(name, logPath.string(), true);
        set_default_logger(logger);

        if (std::filesystem::path dbPath = *foundPath / "TDB.db"; !exists(dbPath)) {
            spdlog::info("{0}", "TDB: Does not exist");
            spdlog::info("{0}", "TDB: Starting creation");

            const auto &api = API::get();
            const auto tdb = api->tdb();
            spdlog::info("{0} {1}", "num_types:", tdb->get_num_types());
            spdlog::info("{0} {1}", "num_fields:", tdb->get_num_fields());
            spdlog::info("{0} {1}", "num_methods:", tdb->get_num_methods());

            const auto [Type, Field, Method] = TDBIdMap::get();
            spdlog::info("{0} {1}", "Type size:", Type.size());
            spdlog::info("{0} {1}", "Field Static size:", Field.Static.size());
            spdlog::info("{0} {1}", "Field Instance size:", Field.Instance.size());
            spdlog::info("{0} {1}", "Method Static size:", Method.Static.size());
            spdlog::info("{0} {1}", "Method Instance size:", Method.Instance.size());

            try {
                SQLite::Database db(":memory:", SQLite::OPEN_READWRITE);
                db.exec(SQLResource::InitSchema);

                SQLite::Statement insertType(db, "INSERT INTO Type (Id, Name, Namespace) VALUES (?, ?, ?)");
                SQLite::Statement insertTypeParentHierarchy(
                    db, "INSERT INTO TypeParentHierarchy (TypeId, ParentId) VALUES (?, ?)");
                SQLite::Statement insertTypeDeclaringHierarchy(
                    db, "INSERT INTO TypeDeclaringHierarchy (TypeId, DeclaringId) VALUES (?, ?)");
                for (const auto &[type, id] : Type) {
                    insertType.bind(1, id);
                    insertType.bind(2, type->get_name());
                    insertType.bind(3, type->get_namespace());
                    insertType.exec();
                    insertType.reset();

                    if (const auto parent_type = type->get_parent_type(); parent_type != nullptr) {
                        if (const auto &typeIterator = Type.find(parent_type); typeIterator != Type.end()) {
                            insertTypeParentHierarchy.bind(1, id);
                            insertTypeParentHierarchy.bind(2, typeIterator->second);
                            insertTypeParentHierarchy.exec();
                            insertTypeParentHierarchy.reset();
                        }
                    }

                    if (const auto declaring_type = type->get_declaring_type(); declaring_type != nullptr) {
                        if (const auto &typeIterator = Type.find(declaring_type); typeIterator != Type.end()) {
                            insertTypeDeclaringHierarchy.bind(1, id);
                            insertTypeDeclaringHierarchy.bind(2, typeIterator->second);
                            insertTypeDeclaringHierarchy.exec();
                            insertTypeDeclaringHierarchy.reset();
                        }
                    }
                }

                SQLite::Statement insertStaticField(db, "INSERT INTO StaticField (Id, Name) VALUES (?, ?)");
                SQLite::Statement insertStaticFieldValueTypeMapping(
                    db, "INSERT INTO StaticFieldValueTypeMapping (FieldId, TypeId) VALUES (?, ?)");
                SQLite::Statement insertTypeStaticFieldAssociation(
                    db, "INSERT INTO TypeStaticFieldAssociation (TypeId, FieldId) VALUES (?, ?)");
                for (const auto &[field, id] : Field.Static) {
                    insertStaticField.bind(1, id);
                    insertStaticField.bind(2, field->get_name());
                    insertStaticField.exec();
                    insertStaticField.reset();

                    if (const auto value_type = field->get_type(); value_type != nullptr) {
                        if (const auto &typeIterator = Type.find(value_type); typeIterator != Type.end()) {
                            insertStaticFieldValueTypeMapping.bind(1, id);
                            insertStaticFieldValueTypeMapping.bind(2, typeIterator->second);
                            insertStaticFieldValueTypeMapping.exec();
                            insertStaticFieldValueTypeMapping.reset();
                        }
                    }

                    if (const auto declaring_type = field->get_declaring_type(); declaring_type != nullptr) {
                        if (const auto &typeIterator = Type.find(declaring_type); typeIterator != Type.end()) {
                            insertTypeStaticFieldAssociation.bind(1, typeIterator->second);
                            insertTypeStaticFieldAssociation.bind(2, id);
                            insertTypeStaticFieldAssociation.exec();
                            insertTypeStaticFieldAssociation.reset();
                        }
                    }
                }

                SQLite::Statement insertField(db, "INSERT INTO Field (Id, Name) VALUES (?, ?)");
                SQLite::Statement insertFieldValueTypeMapping(
                    db, "INSERT INTO FieldValueTypeMapping (FieldId, TypeId) VALUES (?, ?)");
                SQLite::Statement insertTypeFieldAssociation(
                    db, "INSERT INTO TypeFieldAssociation (TypeId, FieldId) VALUES (?, ?)");
                for (const auto &[field, id] : Field.Instance) {
                    insertField.bind(1, id);
                    insertField.bind(2, field->get_name());
                    insertField.exec();
                    insertField.reset();

                    if (const auto value_type = field->get_type(); value_type != nullptr) {
                        if (const auto &typeIterator = Type.find(value_type); typeIterator != Type.end()) {
                            insertFieldValueTypeMapping.bind(1, id);
                            insertFieldValueTypeMapping.bind(2, typeIterator->second);
                            insertFieldValueTypeMapping.exec();
                            insertFieldValueTypeMapping.reset();
                        }
                    }

                    if (const auto declaring_type = field->get_declaring_type(); declaring_type != nullptr) {
                        if (const auto &typeIterator = Type.find(declaring_type); typeIterator != Type.end()) {
                            insertTypeFieldAssociation.bind(1, typeIterator->second);
                            insertTypeFieldAssociation.bind(2, id);
                            insertTypeFieldAssociation.exec();
                            insertTypeFieldAssociation.reset();
                        }
                    }
                }

                SQLite::Statement insertStaticMethod(db, "INSERT INTO StaticMethod (Id, Name) VALUES (?, ?)");
                SQLite::Statement insertStaticMethodReturnTypeMapping(
                    db, "INSERT INTO StaticMethodReturnTypeMapping (MethodId, TypeId) VALUES (?, ?)");
                SQLite::Statement insertTypeStaticMethodAssociation(
                    db, "INSERT INTO TypeStaticMethodAssociation (TypeId, MethodId) VALUES (?, ?)");
                SQLite::Statement insertStaticMethodParameter(
                    db, "INSERT INTO StaticMethodParameter (Id, Name) VALUES (?, ?)");
                SQLite::Statement insertStaticMethodParameterTypeMapping(
                    db, "INSERT INTO StaticMethodParameterTypeMapping (MethodParameterId, TypeId) VALUES (?, ?)");
                SQLite::Statement insertStaticMethodParameterAssociation(
                    db,
                    "INSERT INTO StaticMethodParameterAssociation (MethodId, ParameterId, ParameterIndex) VALUES (?, "
                    "?, ?)");

                int32_t StaticMethodParameterId = 0;
                for (const auto &[method, id] : Method.Instance) {
                    insertStaticMethod.bind(1, id);
                    insertStaticMethod.bind(2, method->get_name());
                    insertStaticMethod.exec();
                    insertStaticMethod.reset();

                    if (const auto return_type = method->get_return_type(); return_type != nullptr) {
                        if (const auto &typeIterator = Type.find(return_type); typeIterator != Type.end()) {
                            insertStaticMethodReturnTypeMapping.bind(1, id);
                            insertStaticMethodReturnTypeMapping.bind(2, typeIterator->second);
                            insertStaticMethodReturnTypeMapping.exec();
                            insertStaticMethodReturnTypeMapping.reset();
                        }
                    }

                    if (const auto declaring_type = method->get_declaring_type(); declaring_type != nullptr) {
                        if (const auto &typeIterator = Type.find(declaring_type); typeIterator != Type.end()) {
                            insertTypeStaticMethodAssociation.bind(1, typeIterator->second);
                            insertTypeStaticMethodAssociation.bind(2, id);
                            insertTypeStaticMethodAssociation.exec();
                            insertTypeStaticMethodAssociation.reset();
                        }
                    }

                    for (const auto params = method->get_params();
                         const auto &[index, param] : std::views::enumerate(params))
                    {
                        insertStaticMethodParameter.bind(1, StaticMethodParameterId);
                        insertStaticMethodParameter.bind(2, param.name);
                        insertStaticMethodParameter.exec();
                        insertStaticMethodParameter.reset();

                        const auto paramType = reinterpret_cast<API::TypeDefinition *>(param.t);
                        if (const auto &typeIterator = Type.find(paramType); typeIterator != Type.end()) {
                            insertStaticMethodParameterTypeMapping.bind(1, StaticMethodParameterId);
                            insertStaticMethodParameterTypeMapping.bind(2, typeIterator->second);
                            insertStaticMethodParameterTypeMapping.exec();
                            insertStaticMethodParameterTypeMapping.reset();
                        }

                        insertStaticMethodParameterAssociation.bind(1, id);
                        insertStaticMethodParameterAssociation.bind(2, StaticMethodParameterId);
                        insertStaticMethodParameterAssociation.bind(3, index);
                        insertStaticMethodParameterAssociation.exec();
                        insertStaticMethodParameterAssociation.reset();

                        StaticMethodParameterId++;
                    }
                }

                SQLite::Statement insertMethod(db, "INSERT INTO Method (Id, Name) VALUES (?, ?)");
                SQLite::Statement insertMethodReturnTypeMapping(
                    db, "INSERT INTO MethodReturnTypeMapping (MethodId, TypeId) VALUES (?, ?)");
                SQLite::Statement insertTypeMethodAssociation(
                    db, "INSERT INTO TypeMethodAssociation (TypeId, MethodId) VALUES (?, ?)");
                SQLite::Statement insertMethodParameter(db, "INSERT INTO MethodParameter (Id, Name) VALUES (?, ?)");
                SQLite::Statement insertMethodParameterTypeMapping(
                    db, "INSERT INTO MethodParameterTypeMapping (MethodParameterId, TypeId) VALUES (?, ?)");
                SQLite::Statement insertMethodParameterAssociation(
                    db,
                    "INSERT INTO MethodParameterAssociation (MethodId, ParameterId, ParameterIndex) VALUES (?, ?, ?)");

                int32_t MethodParameterId = 0;
                for (const auto &[method, id] : Method.Instance) {
                    insertMethod.bind(1, id);
                    insertMethod.bind(2, method->get_name());
                    insertMethod.exec();
                    insertMethod.reset();

                    if (const auto return_type = method->get_return_type(); return_type != nullptr) {
                        if (const auto &typeIterator = Type.find(return_type); typeIterator != Type.end()) {
                            insertMethodReturnTypeMapping.bind(1, id);
                            insertMethodReturnTypeMapping.bind(2, typeIterator->second);
                            insertMethodReturnTypeMapping.exec();
                            insertMethodReturnTypeMapping.reset();
                        }
                    }

                    if (const auto declaring_type = method->get_declaring_type(); declaring_type != nullptr) {
                        if (const auto &typeIterator = Type.find(declaring_type); typeIterator != Type.end()) {
                            insertTypeMethodAssociation.bind(1, typeIterator->second);
                            insertTypeMethodAssociation.bind(2, id);
                            insertTypeMethodAssociation.exec();
                            insertTypeMethodAssociation.reset();
                        }
                    }

                    for (const auto params = method->get_params();
                         const auto &[index, param] : std::views::enumerate(params))
                    {
                        insertMethodParameter.bind(1, MethodParameterId);
                        insertMethodParameter.bind(2, param.name);
                        insertMethodParameter.exec();
                        insertMethodParameter.reset();

                        const auto paramType = reinterpret_cast<API::TypeDefinition *>(param.t);
                        if (const auto &typeIterator = Type.find(paramType); typeIterator != Type.end()) {
                            insertMethodParameterTypeMapping.bind(1, MethodParameterId);
                            insertMethodParameterTypeMapping.bind(2, typeIterator->second);
                            insertMethodParameterTypeMapping.exec();
                            insertMethodParameterTypeMapping.reset();
                        }
                        insertMethodParameterAssociation.bind(1, id);
                        insertMethodParameterAssociation.bind(2, MethodParameterId);
                        insertMethodParameterAssociation.bind(3, index);
                        insertMethodParameterAssociation.exec();
                        insertMethodParameterAssociation.reset();

                        MethodParameterId++;
                    }
                }

                db.backup(dbPath.string().c_str(), SQLite::Database::BackupType::Save);
            } catch (const std::exception &err) {
                spdlog::info("{0} {1}", "ERROR:", err.what());
            } catch (...) {
                spdlog::info("{0} {1}", "ERROR:", "Unknown error");
            }
        } else {
            spdlog::info("{0}", "TDB: Already exists");
        }
    }
    return true;
}
