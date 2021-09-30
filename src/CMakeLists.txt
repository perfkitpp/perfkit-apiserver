add_executable(
        ${PROJECT_NAME}
        main.cpp
        session.cpp
        session.hpp
        crow_body.cpp
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
)
