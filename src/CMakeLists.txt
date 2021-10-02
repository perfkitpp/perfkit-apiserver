add_executable(
        ${PROJECT_NAME}
        main.cpp
        crow_body.cpp
        middlewares/auth.cpp
        middlewares/auth.hpp
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
        cppcrow
        perfkit::_internal::net-provider-protocols
)
