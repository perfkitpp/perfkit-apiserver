add_executable(
        ${PROJECT_NAME}
        main.cpp
        crow_body.cpp
        middlewares/auth.cpp
)

# ============== CONFIGURE TARGET
target_include_directories(
        ${PROJECT_NAME}

        PRIVATE
        ${Boost_INCLUDE_DIR}
)

target_link_libraries(
        ${PROJECT_NAME}

        pthread
        spdlog::spdlog
        perfkit::core
        perfkit::net::core
        perfkit::cli
        cppcrow
        range-v3::range-v3
)
