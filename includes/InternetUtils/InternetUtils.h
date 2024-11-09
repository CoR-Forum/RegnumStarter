// InternetUtils.h
#pragma once

#include <windows.h>
#include <wininet.h>
#include <string>

/**
 * @brief Closes the given internet handles.
 * 
 * @param hRequest Handle to the HTTP request.
 * @param hConnect Handle to the internet connection.
 * @param hInternet Handle to the internet session.
 */
void CloseInternetHandles(HINTERNET hRequest, HINTERNET hConnect, HINTERNET hInternet);

/**
 * @brief Logs a message and closes the given internet handles.
 * 
 * @param message The message to log.
 * @param hRequest Handle to the HTTP request.
 * @param hConnect Handle to the internet connection.
 * @param hInternet Handle to the internet session.
 * @return true if handles were closed successfully, false otherwise.
 */
bool LogAndCloseHandles(const std::string& message, HINTERNET hRequest, HINTERNET hConnect, HINTERNET hInternet);

/**
 * @brief Opens an internet connection.
 * 
 * @return Handle to the internet session.
 */
HINTERNET OpenInternetConnection();

/**
 * @brief Connects to the API using the given internet session handle.
 * 
 * @param hInternet Handle to the internet session.
 * @return Handle to the internet connection.
 */
HINTERNET ConnectToAPI(HINTERNET hInternet);

/**
 * @brief Connects to the API using the given internet session handle.
 * 
 * @param hInternet Handle to the internet session.
 * @return Handle to the internet connection.
 */
HINTERNET ConnectToAPIv2(HINTERNET hInternet);

/**
 * @brief Sends an HTTP request to the given path using the provided connection handle.
 * 
 * @param hConnect Handle to the internet connection.
 * @param path The path to send the request to.
 * @return Handle to the HTTP request.
 */
HINTERNET SendHTTPRequest(HINTERNET hConnect, const std::string& path);

/**
 * @brief Sends an HTTP POST request to the given path using the provided connection handle.
 * 
 * @param hConnect Handle to the internet connection.
 * @param path The path to send the request to.
 * @param payload The payload to send with the request.
 * @return Handle to the HTTP request.
 */
HINTERNET SendHTTPPostRequest(HINTERNET hConnect, const std::string& path, const std::string& requestBody);


/** 
 * @brief Sends an HTTP PUT request to the given path using the provided connection handle.
 * 
 * @param hConnect Handle to the internet connection.
 * @param path The path to send the request to.
 * @param payload The payload to send with the request.
 */
HINTERNET SendHTTPPutRequest(HINTERNET hConnect, const std::string& path, const std::string& requestBody);

/**
 * @brief Reads the response from the given HTTP request handle.
 * 
 * @param hRequest Handle to the HTTP request.
 * @return The response as a string.
 */
std::string ReadResponse(HINTERNET hRequest);

/**
 * @brief Logs a message.
 * 
 * @param message The message to log.
 */
extern void Log(const std::string& message);

/**
 * @brief Logs a debug message.
 * 
 * @param message The debug message to log.
 */
extern void LogDebug(const std::string& message);

/**
 * @brief Fetches data from the given API URL.
 * 
 * @param url The URL to fetch data from.
 * @return The response as a string.
 */
std::string FetchDataFromAPI(const std::string& url);

extern std::string session_id;