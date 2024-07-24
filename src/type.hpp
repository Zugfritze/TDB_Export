#pragma once

#include <memory>
#include <string>
#include <utility>

struct Namespace {
    static int IdCount;
    int Id;
    const std::string Name;
    const std::shared_ptr<Namespace> Parent;

    explicit Namespace(std::string Name) : Id(IdCount++), Name(std::move(Name)), Parent(nullptr) {}
    explicit Namespace(std::string Name, const std::shared_ptr<Namespace> &Parent) :
        Id(IdCount++), Name(std::move(Name)), Parent(Parent) {}
};
int Namespace::IdCount = 0;
