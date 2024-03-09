#include "httplib.h"
#include <filesystem>
#include <fstream>
#include <sstream> 
#include <string>
#include <string_view>
#include <vector>

namespace fs = std::filesystem;
using namespace httplib;

template <typename TP>
std::time_t to_time_t(TP tp)
{
    using namespace std::chrono;
    auto sctp = time_point_cast<system_clock::duration>(tp - TP::clock::now()
              + system_clock::now());
    return system_clock::to_time_t(sctp);
}

static bool ends_with(std::string_view str, std::string_view suffix)
{
    return str.size() >= suffix.size() && str.compare(str.size()-suffix.size(), suffix.size(), suffix) == 0;
}

std::string getLiveFrame() {
  
  const std::string path = "data/stream/";
  std::time_t max_t;
  std::string max_t_filepath;

  max_t = 0;
  for (auto& file : fs::directory_iterator(path)) {
    fs::file_time_type ftime = fs::last_write_time(file.path());
    std::time_t tt = to_time_t(ftime);
    if (tt > max_t) {
      max_t = tt;
      max_t_filepath = file.path();

    }    
  }

  // Read latest frame as buffer stream from file storage
  std::ifstream in(max_t_filepath, std::ios::in | std::ios::binary);
  std::ostringstream contents;
  if(in){
      contents << in.rdbuf();
      in.close();
  }
  return contents.str();
}

std::string getDetectedFrame() {
  
  const std::string path = "data/detection/";
  std::time_t max_t;
  std::string max_t_filepath;
  std::string detected_img_path;

  max_t = 0;
  for (auto& file : fs::directory_iterator(path)) {
    fs::file_time_type ftime = fs::last_write_time(file.path());
    std::time_t tt = to_time_t(ftime);
    if (tt > max_t) {
      max_t = tt;
      max_t_filepath = file.path();
    }    
  }

  std::string detectedFramePath = max_t_filepath;
  for (const auto & entry : fs::directory_iterator(detectedFramePath)) {
    std::string ep = entry.path().string();
    if (ends_with(ep, "jpg")) {
      detected_img_path = entry.path();
      break;
    }
  }

  // Read latest frame as buffer stream from file storage
  std::ifstream in(detected_img_path, std::ios::in | std::ios::binary);
  std::ostringstream contents;
  if(in){
      contents << in.rdbuf();
      in.close();
  }
  return contents.str();
}

int main(void) {
  Server svr;

  svr.Get("/hi", [](const Request & /*req*/, Response &res) {
    res.set_header("Access-Control-Allow-Origin", "*");
    res.set_content("Hello World!", "text/plain");
  });

  svr.Get("/liveframe", [](const Request & /*req*/, Response &res) {
    const std::string live_frame = getLiveFrame();
    res.set_header("Access-Control-Allow-Origin", "*");
    res.set_content(live_frame, "image/jpeg");
  });

  svr.Get("/detected", [](const Request & /*req*/, Response &res) {
    const std::string live_frame = getDetectedFrame();
    res.set_header("Access-Control-Allow-Origin", "*");
    res.set_content(live_frame, "image/jpeg");
  });

  // Handle browser preflight request to check CORS
  svr.Options("/(.*)", [&](const Request & /*req*/, Response &res) {
    res.set_header("Access-Control-Allow-Methods", "*");
    res.set_header("Access-Control-Allow-Headers", "*");
    res.set_header("Access-Control-Allow-Origin", "*");
    res.set_header("Connection", "close");
  });

  svr.listen("0.0.0.0", 8081);
}

