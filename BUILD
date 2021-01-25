load("@rules_cc//cc:defs.bzl", "cc_library")

BAZEL_CXXOPTS  = select({
    ":on_linux": ["-Wall -Wextra -std=c++17"],
    ":on_windows": ["/std:c++17 /bigobj"],
    "//conditions:default": [],
})

INCLUDES = ["include"]

config_setting(
    name = "on_linux",
    constraint_values = [
        "@platforms//os:linux",
    ],
)

config_setting(
    name = "on_windows",
    constraint_values = [
        "@platforms//os:windows",
    ],
)

config_setting(
    name = "with_postgrsql",
    values = {
        "define": "db=postgrsql"
    }
)

config_setting(
    name = "with_sqlite",
    values = {
        "define": "db=sqlite"
    }
)

config_setting(
    name = "example_build",
    values = {
        "define": "EXAMPLE_BUILD"
    }
)

config_setting(
    name = "with_coroutine",
    values = {
        "define": "serv=coro"
    }
)

cc_library(
    name = "fht",
    copts = select({
        "with_coroutine": ["-DWITH_COROUTINE"],
        "//conditions:default": [],
    }) + select({
        ":with_postgrsql": ["-DDBPOSTGRESQL"],
        ":with_sqlite": ["-DDBSQLITE"],
        "//conditions:default": ["-DDBNONE"],
    }) + BAZEL_CXXOPTS,
    srcs = [
        "src/FHT/Common/Controller/Client/Client.cpp",
        "src/FHT/Config/Config.cpp",
    ] + select({
        ":with_coroutine": [
            "src/FHT/Common/Controller/Client/WebClientCoro.cpp",
            "src/FHT/Common/Controller/Server/ServerCoro.cpp",
        ],
        "//conditions:default": [
            "src/FHT/Common/Controller/Client/WebClientAsync.cpp",
            "src/FHT/Common/Controller/Server/ServerAsync.cpp",
        ],
    }),
    hdrs = [
        "include/FHT/Common/Controller/Client/Client.h",
        "include/FHT/Common/Controller/Client/WebClient.h",
        "include/FHT/Common/Controller/Server/Server.h",
        "include/FHT/Common/Controller/Controller.h",
        "include/FHT/Common/Template.h",
        "include/FHT/Interface/Common/iClient.h",
        "include/FHT/Interface/Common/iServer.h",
        "include/FHT/Config/Config.h",
        "include/FHT/CoreFHT.h",
    #-------------------------------------------#
    ] + glob(["boost/**/*.hpp"]),
    #-------------------------------------------#
    includes = INCLUDES,
    deps = [
        ":core",
        ":dbcore",
        "@openssl//:ssl",
        "@boost//:asio_ssl",
        "@boost//:beast",
        "@boost//:system",
        "@boost//:regex",
        "@boost//:serialization",
        "@boost//:thread",
        "@boost//:chrono",
    ] + select({
        "with_coroutine": [
            "@boost//:context",
            "@boost//:coroutine",
            "@boost//:program_options",
        ],
        "//conditions:default": [],
    }),
    visibility = ["//visibility:public"],
)

cc_library(
    name = "core",
    srcs = [
        "src/FHT/Common/Controller/Hendler/Hendler.cpp",
        "src/FHT/Common/Controller/Logger/Logger.cpp",
        "src/FHT/Common/Controller/Task/Task.cpp",
    ],
    includes = INCLUDES,
    hdrs = [
        "include/FHT/Common/Controller/Hendler/Hendler.h",
        "include/FHT/Common/Controller/Logger/Logger.h",
        "include/FHT/Common/Controller/Task/Task.h",
        "include/FHT/Interface/Common/iHendler.h",
        "include/FHT/Interface/Common/iLogger.h",
        "include/FHT/Interface/Common/iTask.h",
        "include/FHT/LoggerStream.h",
    ],
    deps = [
        "@boost//:beast",
    ],
    copts = BAZEL_CXXOPTS,
    visibility = ["//visibility:public"],
)

cc_library(
    name = "dbcore",
    copts = select({
            ":with_postgrsql": ["-DDBPOSTGRESQL"],
            ":with_sqlite": ["-DDBSQLITE"],
            "//conditions:default": ["-DDBNONE"],
        }) + BAZEL_CXXOPTS,
    srcs = [
        "src/FHT/Common/DBFacade/DBFacade.cpp",
    ],
    includes = INCLUDES,
    hdrs = [
        "include/FHT/Common/DBFacade/DBFacade.h",
        "include/FHT/Interface/DBFacade/iDBFacade.h",
    ],
    deps = select({
        ":with_postgrsql": [":postgrsql"],
        ":with_sqlite": [":sqlite"],
        "//conditions:default": [],
    }),
    visibility = ["//visibility:public"],
)

cc_library(
    name = "postgrsql",
    srcs = [
        "src/FHT/Common/DBFacade/Postgrsql/PostgresConnection.cpp",
        "src/FHT/Common/DBFacade/Postgrsql/Postgresql.cpp",
    ],
    includes = INCLUDES,
    hdrs = [
        "include/FHT/Common/DBFacade/Postgrsql/PostgresConnection.h",
        "include/FHT/Common/DBFacade/Postgrsql/Postgresql.h",
        "include/FHT/Interface/DBFacade/postgresConnection.h",
    ],
    copts = BAZEL_CXXOPTS,
    visibility = ["//visibility:public"],
)

cc_library(
    name = "sqlite",
    srcs = [
        "src/FHT/Common/DBFacade/SQLite/SQLite.cpp",
        "src/FHT/Common/DBFacade/SQLite/sqliteConnection.cpp",
    ],
    includes = INCLUDES,
    hdrs = [
        "include/FHT/Common/DBFacade/SQLite/SQLite.h",
        "include/FHT/Common/DBFacade/SQLite/sqliteConnection.h",
        "include/FHT/Interface/DBFacade/sqliteConnection.h",
    ],
    copts = BAZEL_CXXOPTS,
    visibility = ["//visibility:public"],
)

