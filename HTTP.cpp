#include "HTTP.hpp"

namespace HTTP
{
    std::string userAgent = "WinHTTP/1.0";
    INTERNET_PORT port = INTERNET_DEFAULT_HTTPS_PORT; // 443
    DWORD requestFlags = INTERNET_FLAG_SECURE | defaultNoCacheFlags | defaultBaseFlags;
    std::string contentType = "application/x-www-form-urlencoded";

    byte* Post(std::string URL, std::string input, DWORD* bytesRead, void (*progressCallback)(void*, float), void* callbackData)
    {
        progressCallback(callbackData, 0.f);


        // split string into host and directory
        // [ garbage ][     host part    ][     directory part     ]
        //   https://    www.example.com     /subdirectory/api.php

        // remove protocol from url
        if (URL.rfind("http://") == 0)
        {
            URL = URL.substr(7);
        }
        else if (URL.rfind("https://") == 0)
        {
            URL = URL.substr(8);
        }

        // split host and directory
        int directoryIndex = URL.find("/");
        std::string host = URL;
        std::string directory = "";
        if (directoryIndex >= 0)
        {
            host = URL.substr(0, directoryIndex);
            directory = URL.substr(directoryIndex + 1);
        }
        progressCallback(callbackData, 0.01f);

        HINTERNET hInternet = InternetOpen(
            userAgent.c_str(),
            INTERNET_OPEN_TYPE_PRECONFIG,
            NULL, NULL,
            0
        );
        if (!hInternet)
        {
            std::cout << "a" << std::endl;
            return nullptr;
        }

        HINTERNET hConnection = InternetConnect(
            hInternet,
            host.c_str(),
            INTERNET_DEFAULT_HTTPS_PORT,
            NULL, NULL,
            INTERNET_SERVICE_HTTP,
            0, 0
        );
        if (!hConnection)
        {
            std::cout << "b" << std::endl;
            return nullptr;
        }

        // "Long pointer to a null-terminated array of string pointers indicating content types accepted by the client"
        LPCSTR acceptContentTypes[]{ "*/*", 0 };

        HINTERNET hRequest = HttpOpenRequest(hConnection,
            "POST",
            directory.c_str(),
            "HTTP/1.1",
            0,
            acceptContentTypes,
            requestFlags,
            0
        );
        if (!hRequest)
        {
            std::cout << "c" << std::endl;
            return nullptr;
        }

        bool headersAdded = HttpAddRequestHeaders(
            hRequest,
            ("Content-Type: " + contentType + "\r\n").c_str(),
            48,
            HTTP_ADDREQ_FLAG_REPLACE
        );
        progressCallback(callbackData, 0.05f);

        bool requestSuccess = HttpSendRequest(hRequest,
            // headers and length, not supported yet
            NULL, 0,
            // post data and post length
            (void*)input.c_str(), input.size()
        );

        if (!requestSuccess)
        {
            std::cout << "e" << std::endl;
            return nullptr;
        }
        progressCallback(callbackData, 0.1f);

        // dynamic buffer to hold file
        byte* fileBuffer = NULL;
        DWORD fileSize = 0;
        DWORD predictedFileSize = 0;
        DWORD dwordSize = sizeof(predictedFileSize);
        HttpQueryInfo(hRequest, HTTP_QUERY_CONTENT_LENGTH, &predictedFileSize, &dwordSize, NULL); // if it fails then whatever

        // buffer for each kb of the file
        byte* chunkBuffer = (byte*)malloc(1024);
        DWORD chunkSize = 1024;
        DWORD chunkBytesRead = 0;
        if (!chunkBuffer)
        {
            free(fileBuffer);
            std::cout << "f" << std::endl;
            return nullptr;
        }

        do
        {
            chunkBytesRead = 0;
            InternetReadFile(hRequest, chunkBuffer, chunkSize, &chunkBytesRead);

            byte* re = (byte*)realloc(fileBuffer, fileSize + chunkBytesRead);
            if (!re)
            {
                free(fileBuffer);
                free(chunkBuffer);
                std::cout << "g" << std::endl;
                return nullptr;
            }
            else
            {
                fileBuffer = re;
            }
            memcpy(fileBuffer + fileSize, chunkBuffer, chunkBytesRead);
            fileSize += chunkBytesRead;
            if (predictedFileSize > 0 && predictedFileSize >= fileSize)
                progressCallback(callbackData, (float)fileSize / (float)predictedFileSize);
            else
                progressCallback(callbackData, 0.99f);
        } while (chunkBytesRead > 0);
        free(chunkBuffer);

        if (fileSize == 0)
        {
            std::cout << "h" << std::endl;
            progressCallback(callbackData, 0.f);
            free(fileBuffer);
            return nullptr;
        }
        if (bytesRead)
            *bytesRead = fileSize;

        progressCallback(callbackData, 1.f);
        return fileBuffer;
    }
}