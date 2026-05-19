cmake_minimum_required(VERSION 3.21)
project(Legalyze LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network)

set(SOURCES
    src/main.cpp
    src/app/AppController.cpp
    src/app/SessionManager.cpp
    src/app/Router.cpp
    src/api/ApiClient.cpp
    src/api/AuthApi.cpp
    src/api/ChatApi.cpp
    src/api/DocumentApi.cpp
    src/api/KnowledgeBaseApi.cpp
    src/viewmodels/LoginViewModel.cpp
    src/viewmodels/ChatViewModel.cpp
    src/viewmodels/ConversationsViewModel.cpp
    src/viewmodels/DocumentsViewModel.cpp
    src/viewmodels/KnowledgeBaseViewModel.cpp
    src/widgets/MainWindow.cpp
    src/widgets/LoginPage.cpp
    src/widgets/ChatPage.cpp
    src/widgets/DocumentsPage.cpp
    src/widgets/KnowledgeBasePage.cpp
    src/widgets/SettingsPage.cpp
    src/widgets/CitationWidget.cpp
    src/widgets/MessageBubbleWidget.cpp
    src/network/SseClient.cpp
    src/network/NetworkUtils.cpp
    src/storage/TokenStorage.cpp
    src/storage/LocalCache.cpp
    src/utils/JsonUtils.cpp
    src/utils/DateTimeUtils.cpp
)

set(HEADERS
    src/app/AppController.h
    src/app/SessionManager.h
    src/app/Router.h
    src/api/ApiClient.h
    src/api/AuthApi.h
    src/api/ChatApi.h
    src/api/DocumentApi.h
    src/api/KnowledgeBaseApi.h
    src/models/User.h
    src/models/Conversation.h
    src/models/Message.h
    src/models/DocumentItem.h
    src/models/Citation.h
    src/models/RagAnswer.h
    src/viewmodels/LoginViewModel.h
    src/viewmodels/ChatViewModel.h
    src/viewmodels/ConversationsViewModel.h
    src/viewmodels/DocumentsViewModel.h
    src/viewmodels/KnowledgeBaseViewModel.h
    src/widgets/MainWindow.h
    src/widgets/LoginPage.h
    src/widgets/ChatPage.h
    src/widgets/DocumentsPage.h
    src/widgets/KnowledgeBasePage.h
    src/widgets/SettingsPage.h
    src/widgets/CitationWidget.h
    src/widgets/MessageBubbleWidget.h
    src/network/SseClient.h
    src/network/NetworkUtils.h
    src/storage/TokenStorage.h
    src/storage/LocalCache.h
    src/utils/JsonUtils.h
    src/utils/DateTimeUtils.h
)

qt_add_executable(Legalyze
    ${SOURCES}
    ${HEADERS}
)

target_include_directories(Legalyze PRIVATE src)

target_link_libraries(Legalyze PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
)
