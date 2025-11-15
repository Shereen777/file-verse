#ifndef SOCKET_HANDLER_HPP
#define SOCKET_HANDLER_HPP

#include "../include/odf_types.hpp"
#include "Session_Instance.hpp"
#include <nlohmann/json.hpp>
#include <string>
#include <map>
#include "FileSystem.hpp"
#include "file_system.cpp"


using json = nlohmann::json;
using namespace std;

class SocketHandler {
private:
    OMNIInstance* instance;
    
    // Validation helpers
    bool validate_session(const string& session_id, Session*& out_session) {
        for (auto* sess : instance->sessions) {
            if (sess->session_id == session_id) {
                out_session = sess;
                return true;
            }
        }
        return false;
    }
    
    bool validate_path(const string& path) {
        if (path.empty() || path[0] != '/') return false;
        if (path.length() > 256) return false;
        // Check for invalid characters
        for (char c : path) {
            if (c == '\0' || c == '\n' || c == '\r') return false;
        }
        return true;
    }
    
    bool validate_size(size_t size) {
        return size <= 1048576; // 1MB max per operation
    }
    
    json create_success_response(const string& operation, const string& request_id, const json& data = json::object()) {
        return {
            {"status", "success"},
            {"operation", operation},
            {"request_id", request_id},
            {"data", data}
        };
    }
    
    json create_error_response(const string& operation, const string& request_id, int error_code, const string& error_msg) {
        return {
            {"status", "error"},
            {"operation", operation},
            {"request_id", request_id},
            {"error_code", error_code},
            {"error_message", error_msg}
        };
    }
    
    // Operation handlers
    json handle_login(const json& req) {
        try {
            uint32_t user_index = req["parameters"]["user_index"];
            string password = req["parameters"]["password"];
            string request_id = req["request_id"];
            
            void* session = nullptr;
            int result = user_login(&session, instance, user_index, password.c_str());
            
            if (result != static_cast<int>(OFSErrorCodes::SUCCESS)) {
                return create_error_response("login", request_id, result, get_error_message(result));
            }
            
            Session* sess = static_cast<Session*>(session);
            json data = {
                {"session_id", sess->session_id},
                {"username", sess->user->username},
                {"role", static_cast<int>(sess->user->role)}
            };
            
            return create_success_response("login", request_id, data);
        } catch (const exception& e) {
            return create_error_response("login", req["request_id"], 
                static_cast<int>(OFSErrorCodes::ERROR_INVALID_OPERATION), e.what());
        }
    }
    
    json handle_logout(const json& req) {
        try {
            string session_id = req["parameters"]["session_id"];
            string request_id = req["request_id"];
            
            Session* sess = nullptr;
            if (!validate_session(session_id, sess)) {
                return create_error_response("logout", request_id,
                    static_cast<int>(OFSErrorCodes::ERROR_INVALID_SESSION), "Session not found");
            }
            
            int result = user_logout(sess);
            if (result != static_cast<int>(OFSErrorCodes::SUCCESS)) {
                return create_error_response("logout", request_id, result, get_error_message(result));
            }
            
            return create_success_response("logout", request_id, {{"message", "Logged out successfully"}});
        } catch (const exception& e) {
            return create_error_response("logout", req["request_id"],
                static_cast<int>(OFSErrorCodes::ERROR_INVALID_OPERATION), e.what());
        }
    }
    
    json handle_file_create(const json& req) {
        try {
            string session_id = req["parameters"]["session_id"];
            string path = req["parameters"]["path"];
            string data_str = req["parameters"].value("data", "");
            string request_id = req["request_id"];
            
            if (!validate_path(path)) {
                return create_error_response("file_create", request_id,
                    static_cast<int>(OFSErrorCodes::ERROR_INVALID_PATH), "Invalid path");
            }
            
            if (!validate_size(data_str.length())) {
                return create_error_response("file_create", request_id,
                    static_cast<int>(OFSErrorCodes::ERROR_NO_SPACE), "Data too large");
            }
            
            Session* sess = nullptr;
            if (!validate_session(session_id, sess)) {
                return create_error_response("file_create", request_id,
                    static_cast<int>(OFSErrorCodes::ERROR_INVALID_SESSION), "Invalid session");
            }
            
            int result = file_create(sess, path.c_str(), data_str.c_str(), data_str.length());
            if (result != static_cast<int>(OFSErrorCodes::SUCCESS)) {
                return create_error_response("file_create", request_id, result, get_error_message(result));
            }
            
            return create_success_response("file_create", request_id, 
                {{"path", path}, {"size", data_str.length()}});
        } catch (const exception& e) {
            return create_error_response("file_create", req["request_id"],
                static_cast<int>(OFSErrorCodes::ERROR_INVALID_OPERATION), e.what());
        }
    }
    
    json handle_file_read(const json& req) {
        try {
            string session_id = req["parameters"]["session_id"];
            string path = req["parameters"]["path"];
            string request_id = req["request_id"];
            
            if (!validate_path(path)) {
                return create_error_response("file_read", request_id,
                    static_cast<int>(OFSErrorCodes::ERROR_INVALID_PATH), "Invalid path");
            }
            
            Session* sess = nullptr;
            if (!validate_session(session_id, sess)) {
                return create_error_response("file_read", request_id,
                    static_cast<int>(OFSErrorCodes::ERROR_INVALID_SESSION), "Invalid session");
            }
            
            char* buffer = nullptr;
            size_t size = 0;
            int result = file_read(sess, path.c_str(), &buffer, &size);
            
            if (result != static_cast<int>(OFSErrorCodes::SUCCESS)) {
                return create_error_response("file_read", request_id, result, get_error_message(result));
            }
            
            string content(buffer, size);
            free_buffer(buffer);
            
            return create_success_response("file_read", request_id,
                {{"content", content}, {"size", size}, {"path", path}});
        } catch (const exception& e) {
            return create_error_response("file_read", req["request_id"],
                static_cast<int>(OFSErrorCodes::ERROR_INVALID_OPERATION), e.what());
        }
    }
    
    json handle_file_delete(const json& req) {
        try {
            string session_id = req["parameters"]["session_id"];
            string path = req["parameters"]["path"];
            string request_id = req["request_id"];
            
            if (!validate_path(path)) {
                return create_error_response("file_delete", request_id,
                    static_cast<int>(OFSErrorCodes::ERROR_INVALID_PATH), "Invalid path");
            }
            
            Session* sess = nullptr;
            if (!validate_session(session_id, sess)) {
                return create_error_response("file_delete", request_id,
                    static_cast<int>(OFSErrorCodes::ERROR_INVALID_SESSION), "Invalid session");
            }
            
            int result = file_delete(sess, path.c_str());
            if (result != static_cast<int>(OFSErrorCodes::SUCCESS)) {
                return create_error_response("file_delete", request_id, result, get_error_message(result));
            }
            
            return create_success_response("file_delete", request_id, {{"path", path}});
        } catch (const exception& e) {
            return create_error_response("file_delete", req["request_id"],
                static_cast<int>(OFSErrorCodes::ERROR_INVALID_OPERATION), e.what());
        }
    }
    
    json handle_file_edit(const json& req) {
        try {
            string session_id = req["parameters"]["session_id"];
            string path = req["parameters"]["path"];
            string data_str = req["parameters"]["data"];
            uint32_t index = req["parameters"]["index"];
            string request_id = req["request_id"];
            
            if (!validate_path(path)) {
                return create_error_response("file_edit", request_id,
                    static_cast<int>(OFSErrorCodes::ERROR_INVALID_PATH), "Invalid path");
            }
            
            if (!validate_size(data_str.length())) {
                return create_error_response("file_edit", request_id,
                    static_cast<int>(OFSErrorCodes::ERROR_NO_SPACE), "Data too large");
            }
            
            Session* sess = nullptr;
            if (!validate_session(session_id, sess)) {
                return create_error_response("file_edit", request_id,
                    static_cast<int>(OFSErrorCodes::ERROR_INVALID_SESSION), "Invalid session");
            }
            
            int result = file_edit(sess, path.c_str(), data_str.c_str(), data_str.length(), index);
            if (result != static_cast<int>(OFSErrorCodes::SUCCESS)) {
                return create_error_response("file_edit", request_id, result, get_error_message(result));
            }
            
            return create_success_response("file_edit", request_id,
                {{"path", path}, {"index", index}, {"size", data_str.length()}});
        } catch (const exception& e) {
            return create_error_response("file_edit", req["request_id"],
                static_cast<int>(OFSErrorCodes::ERROR_INVALID_OPERATION), e.what());
        }
    }
    
    json handle_dir_list(const json& req) {
        try {
            string session_id = req["parameters"]["session_id"];
            string path = req["parameters"]["path"];
            string request_id = req["request_id"];
            
            if (!validate_path(path)) {
                return create_error_response("dir_list", request_id,
                    static_cast<int>(OFSErrorCodes::ERROR_INVALID_PATH), "Invalid path");
            }
            
            Session* sess = nullptr;
            if (!validate_session(session_id, sess)) {
                return create_error_response("dir_list", request_id,
                    static_cast<int>(OFSErrorCodes::ERROR_INVALID_SESSION), "Invalid session");
            }
            
            FileEntry* entries = nullptr;
            int count = 0;
            int result = dir_list(sess, path.c_str(), &entries, &count);
            
            if (result != static_cast<int>(OFSErrorCodes::SUCCESS)) {
                return create_error_response("dir_list", request_id, result, get_error_message(result));
            }
            
            json entries_array = json::array();
            for (int i = 0; i < count; i++) {
                entries_array.push_back({
                    {"name", entries[i].name},
                    {"type", entries[i].type},
                    {"size", entries[i].size},
                    {"owner", entries[i].owner},
                    {"permissions", entries[i].permissions}
                });
            }
            free_buffer(entries);
            
            return create_success_response("dir_list", request_id,
                {{"path", path}, {"entries", entries_array}, {"count", count}});
        } catch (const exception& e) {
            return create_error_response("dir_list", req["request_id"],
                static_cast<int>(OFSErrorCodes::ERROR_INVALID_OPERATION), e.what());
        }
    }
    
    json handle_get_stats(const json& req) {
        try {
            string session_id = req["parameters"]["session_id"];
            string request_id = req["request_id"];
            
            Session* sess = nullptr;
            if (!validate_session(session_id, sess)) {
                return create_error_response("get_stats", request_id,
                    static_cast<int>(OFSErrorCodes::ERROR_INVALID_SESSION), "Invalid session");
            }
            
            FSStats stats{};
            int result = get_stats(sess, &stats);
            
            if (result != static_cast<int>(OFSErrorCodes::SUCCESS)) {
                return create_error_response("get_stats", request_id, result, get_error_message(result));
            }
            
            return create_success_response("get_stats", request_id, {
                {"total_size", stats.total_size},
                {"used_space", stats.used_space},
                {"free_space", stats.free_space},
                {"total_files", stats.total_files},
                {"total_directories", stats.total_directories},
                {"total_users", stats.total_users},
                {"active_sessions", stats.active_sessions},
                {"fragmentation", stats.fragmentation}
            });
        } catch (const exception& e) {
            return create_error_response("get_stats", req["request_id"],
                static_cast<int>(OFSErrorCodes::ERROR_INVALID_OPERATION), e.what());
        }
    }
    
public:
    SocketHandler(OMNIInstance* inst) : instance(inst) {}
    
    json process_request(const json& req) {
        try {
            if (!req.contains("operation") || !req.contains("request_id")) {
                return create_error_response("unknown", "invalid",
                    static_cast<int>(OFSErrorCodes::ERROR_INVALID_OPERATION), 
                    "Missing operation or request_id");
            }
            
            string operation = req["operation"];
            
            if (operation == "login") return handle_login(req);
            if (operation == "logout") return handle_logout(req);
            if (operation == "file_create") return handle_file_create(req);
            if (operation == "file_read") return handle_file_read(req);
            if (operation == "file_delete") return handle_file_delete(req);
            if (operation == "file_edit") return handle_file_edit(req);
            if (operation == "dir_list") return handle_dir_list(req);
            if (operation == "get_stats") return handle_get_stats(req);
            
            return create_error_response(operation, req["request_id"],
                static_cast<int>(OFSErrorCodes::ERROR_NOT_IMPLEMENTED), 
                "Operation not implemented");
        } catch (const json::exception& e) {
            return create_error_response("unknown", "invalid",
                static_cast<int>(OFSErrorCodes::ERROR_INVALID_OPERATION), 
                string("JSON parsing error: ") + e.what());
        }
    }
};

#endif