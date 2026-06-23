#include "crow.h"
#include "json.hpp"
#include <curl/curl.h>
#include <cstdlib>
#include <iostream>
#include <string>

using json = nlohmann::json;

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
    size_t totalSize = size * nmemb;
    output->append(static_cast<char*>(contents), totalSize);
    return totalSize;
}

std::string buildAnalysisPrompt(const std::string& idea) {
    return R"(You are a startup analyst. Analyze the following startup idea and provide a detailed response in valid JSON format with exactly these keys:

{
  "swot": {
    "strengths": ["strength1", "strength2", "strength3"],
    "weaknesses": ["weakness1", "weakness2", "weakness3"],
    "opportunities": ["opportunity1", "opportunity2", "opportunity3"],
    "threats": ["threat1", "threat2", "threat3"]
  },
  "market_potential": {
    "tam": "Total Addressable Market estimate",
    "growth_rate": "estimated CAGR percentage",
    "target_audience": "description of ideal customers",
    "revenue_model": "suggested revenue model",
    "score": 7
  },
  "competitors": [
    {"name": "Competitor1", "url": "example.com", "differentiator": "what makes your idea different from them"},
    {"name": "Competitor2", "url": "example.com", "differentiator": "what makes your idea different from them"},
    {"name": "Competitor3", "url": "example.com", "differentiator": "what makes your idea different from them"}
  ],
  "feedback": {
    "overall_score": 7,
    "verdict": "Promising / Risky / Strong / Niche",
    "improvements": ["improvement1", "improvement2", "improvement3"],
    "quick_wins": ["quick win 1", "quick win 2"],
    "summary": "2-3 sentence overall summary of the idea"
  }
}

Startup idea: )" + idea + R"(

Respond ONLY with the JSON object, no markdown, no explanation.)";
}

std::string callOllamaAPI(const std::string& idea, const std::string& modelName) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        return "";
    }

    json requestBody = {
        {"model", modelName},
        {"prompt", buildAnalysisPrompt(idea)},
        {"stream", false},
        {"format", "json"},
        {"options", {
            {"temperature", 0.7}
        }}
    };

    std::string requestStr = requestBody.dump();
    std::string response;

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, "http://127.0.0.1:11434/api/generate");
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, requestStr.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 120L);

    CURLcode res = curl_easy_perform(curl);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        std::cerr << "cURL error: " << curl_easy_strerror(res) << std::endl;
        return "";
    }

    return response;
}

std::string extractContent(const std::string& ollamaResponse) {
    try {
        auto parsed = json::parse(ollamaResponse);
        if (parsed.contains("response") && parsed["response"].is_string()) {
            return parsed["response"].get<std::string>();
        }
        if (parsed.contains("error")) {
            std::cerr << "Ollama error: " << parsed["error"] << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Parse error: " << e.what() << std::endl;
    }
    return "";
}

std::string getModelName() {
    const char* envModel = std::getenv("OLLAMA_MODEL");
    if (envModel && *envModel) {
        return std::string(envModel);
    }
    return "llama3.2:3b";
}

int main() {
    curl_global_init(CURL_GLOBAL_DEFAULT);

    const std::string modelName = getModelName();
    crow::SimpleApp app;

    auto addCORS = [](crow::response& res) {
        res.add_header("Access-Control-Allow-Origin", "*");
        res.add_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
        res.add_header("Access-Control-Allow-Headers", "Content-Type");
    };

    CROW_ROUTE(app, "/api/validate")
    .methods(crow::HTTPMethod::OPTIONS)
    ([&addCORS](const crow::request&) {
        crow::response res(204);
        addCORS(res);
        return res;
    });

    CROW_ROUTE(app, "/api/health")
    ([&addCORS, &modelName]() {
        crow::response res(200);
        addCORS(res);
        res.set_header("Content-Type", "application/json");
        json body = {
            {"status", "ok"},
            {"message", "Startup Validator API running"},
            {"provider", "ollama"},
            {"model", modelName}
        };
        res.body = body.dump();
        return res;
    });

    CROW_ROUTE(app, "/api/validate")
    .methods(crow::HTTPMethod::POST)
    ([&addCORS, &modelName](const crow::request& req) {
        crow::response res;
        addCORS(res);
        res.set_header("Content-Type", "application/json");

        std::string idea;
        try {
            auto body = json::parse(req.body);
            if (!body.contains("idea") || body["idea"].get<std::string>().empty()) {
                res.code = 400;
                res.body = R"({"error":"Please provide a startup idea."})";
                return res;
            }
            idea = body["idea"].get<std::string>();
        } catch (...) {
            res.code = 400;
            res.body = R"({"error":"Invalid JSON body."})";
            return res;
        }

        std::cout << "Validating idea: " << idea.substr(0, 60) << "..." << std::endl;

        std::string ollamaRaw = callOllamaAPI(idea, modelName);
        if (ollamaRaw.empty()) {
            res.code = 500;
            res.body = R"({"error":"Could not reach Ollama. Start Ollama and make sure a local model is installed."})";
            return res;
        }

        std::string content = extractContent(ollamaRaw);
        if (content.empty()) {
            res.code = 500;
            res.body = R"({"error":"Ollama returned an empty or invalid response. Try pulling the configured model again."})";
            return res;
        }

        try {
            auto parsed = json::parse(content);
            res.code = 200;
            res.body = parsed.dump();
        } catch (...) {
            json errObj = {{"error", "AI returned non-JSON"}, {"raw", content}};
            res.code = 500;
            res.body = errObj.dump();
        }

        return res;
    });

    std::cout << "========================================" << std::endl;
    std::cout << " Startup Validator Backend (Ollama)     " << std::endl;
    std::cout << " Listening on http://localhost:8080     " << std::endl;
    std::cout << " Model: " << modelName << std::endl;
    std::cout << "========================================" << std::endl;

    app.port(8080).multithreaded().run();

    curl_global_cleanup();
    return 0;
}
