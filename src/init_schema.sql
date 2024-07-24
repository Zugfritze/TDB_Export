CREATE TABLE IF NOT EXISTS Type
(
    Id        INTEGER PRIMARY KEY,
    Name      TEXT NOT NULL,
    Namespace TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS TypeParentHierarchy
(
    TypeId   INTEGER UNIQUE NOT NULL,
    ParentId INTEGER        NOT NULL,
    CHECK (TypeId != ParentId),
    FOREIGN KEY (TypeId) REFERENCES Type (Id),
    FOREIGN KEY (ParentId) REFERENCES Type (Id),
    PRIMARY KEY (TypeId, ParentId)
);

CREATE TABLE IF NOT EXISTS TypeDeclaringHierarchy
(
    TypeId      INTEGER UNIQUE NOT NULL,
    DeclaringId INTEGER        NOT NULL,
    CHECK (TypeId != DeclaringId),
    FOREIGN KEY (TypeId) REFERENCES Type (Id),
    FOREIGN KEY (DeclaringId) REFERENCES Type (Id),
    PRIMARY KEY (TypeId, DeclaringId)
);

CREATE TABLE IF NOT EXISTS Field
(
    Id   INTEGER PRIMARY KEY,
    Name TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS StaticField
(
    Id   INTEGER PRIMARY KEY,
    Name TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS FieldValueTypeMapping
(
    FieldId INTEGER UNIQUE NOT NULL,
    TypeId  INTEGER        NOT NULL,
    FOREIGN KEY (FieldId) REFERENCES Field (Id),
    FOREIGN KEY (TypeId) REFERENCES Type (Id),
    PRIMARY KEY (FieldId, TypeId)
);

CREATE TABLE IF NOT EXISTS StaticFieldValueTypeMapping
(
    FieldId INTEGER UNIQUE NOT NULL,
    TypeId  INTEGER        NOT NULL,
    FOREIGN KEY (FieldId) REFERENCES StaticField (Id),
    FOREIGN KEY (TypeId) REFERENCES Type (Id),
    PRIMARY KEY (FieldId, TypeId)
);

CREATE TABLE IF NOT EXISTS Method
(
    Id   INTEGER PRIMARY KEY,
    Name TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS StaticMethod
(
    Id   INTEGER PRIMARY KEY,
    Name TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS MethodReturnTypeMapping
(
    MethodId INTEGER UNIQUE NOT NULL,
    TypeId   INTEGER        NOT NULL,
    FOREIGN KEY (MethodId) REFERENCES Method (Id),
    FOREIGN KEY (TypeId) REFERENCES Type (Id),
    PRIMARY KEY (MethodId, TypeId)
);

CREATE TABLE IF NOT EXISTS StaticMethodReturnTypeMapping
(
    MethodId INTEGER UNIQUE NOT NULL,
    TypeId   INTEGER        NOT NULL,
    FOREIGN KEY (MethodId) REFERENCES StaticMethod (Id),
    FOREIGN KEY (TypeId) REFERENCES Type (Id),
    PRIMARY KEY (MethodId, TypeId)
);

CREATE TABLE IF NOT EXISTS MethodParameter
(
    Id   INTEGER PRIMARY KEY,
    Name TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS StaticMethodParameter
(
    Id   INTEGER PRIMARY KEY,
    Name TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS MethodParameterTypeMapping
(
    MethodParameterId INTEGER UNIQUE NOT NULL,
    TypeId            INTEGER        NOT NULL,
    FOREIGN KEY (MethodParameterId) REFERENCES MethodParameter (Id),
    FOREIGN KEY (TypeId) REFERENCES Type (Id),
    PRIMARY KEY (MethodParameterId, TypeId)
);

CREATE TABLE IF NOT EXISTS StaticMethodParameterTypeMapping
(
    MethodParameterId INTEGER UNIQUE NOT NULL,
    TypeId            INTEGER        NOT NULL,
    FOREIGN KEY (MethodParameterId) REFERENCES StaticMethodParameter (Id),
    FOREIGN KEY (TypeId) REFERENCES Type (Id),
    PRIMARY KEY (MethodParameterId, TypeId)
);

CREATE TABLE IF NOT EXISTS TypeFieldAssociation
(
    TypeId  INTEGER NOT NULL,
    FieldId INTEGER NOT NULL,
    FOREIGN KEY (TypeId) REFERENCES Type (Id),
    FOREIGN KEY (FieldId) REFERENCES Field (Id),
    PRIMARY KEY (TypeId, FieldId)
);

CREATE TABLE IF NOT EXISTS TypeStaticFieldAssociation
(
    TypeId  INTEGER NOT NULL,
    FieldId INTEGER NOT NULL,
    FOREIGN KEY (TypeId) REFERENCES Type (Id),
    FOREIGN KEY (FieldId) REFERENCES StaticField (Id),
    PRIMARY KEY (TypeId, FieldId)
);

CREATE TABLE IF NOT EXISTS TypeMethodAssociation
(
    TypeId   INTEGER NOT NULL,
    MethodId INTEGER NOT NULL,
    FOREIGN KEY (TypeId) REFERENCES Type (Id),
    FOREIGN KEY (MethodId) REFERENCES Method (Id),
    PRIMARY KEY (TypeId, MethodId)
);

CREATE TABLE IF NOT EXISTS TypeStaticMethodAssociation
(
    TypeId   INTEGER NOT NULL,
    MethodId INTEGER NOT NULL,
    FOREIGN KEY (TypeId) REFERENCES Type (Id),
    FOREIGN KEY (MethodId) REFERENCES StaticMethod (Id),
    PRIMARY KEY (TypeId, MethodId)
);

CREATE TABLE IF NOT EXISTS MethodParameterAssociation
(
    MethodId       INTEGER        NOT NULL,
    ParameterId    INTEGER UNIQUE NOT NULL,
    ParameterIndex INTEGER        NOT NULL,
    UNIQUE (MethodId, ParameterIndex),
    FOREIGN KEY (MethodId) REFERENCES Method (Id),
    FOREIGN KEY (ParameterId) REFERENCES MethodParameter (Id),
    PRIMARY KEY (MethodId, ParameterId, ParameterIndex)
);

CREATE TABLE IF NOT EXISTS StaticMethodParameterAssociation
(
    MethodId       INTEGER        NOT NULL,
    ParameterId    INTEGER UNIQUE NOT NULL,
    ParameterIndex INTEGER        NOT NULL,
    UNIQUE (MethodId, ParameterIndex),
    FOREIGN KEY (MethodId) REFERENCES StaticMethod (Id),
    FOREIGN KEY (ParameterId) REFERENCES StaticMethodParameter (Id),
    PRIMARY KEY (MethodId, ParameterId, ParameterIndex)
);
