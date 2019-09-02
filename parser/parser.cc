// 数据处理模块
// 把boost文档中涉及到的html进行处：
//    1.去标签
//    2.把文件进行合并   
//      把文档中涉及到的N个HTML的内容合并成一个行文本文件.
//      生成的结果是一个大文件, 里面包含很多行,每一行对应boost
//      文档中的一个html, 这么做的目的是为了让后面的索引模块处理起来更方便
//    3.对文档的结构进行分析，提取出文档的标题，正文，目标url

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include "../common/util.hpp"

const std::string g_input_path = "../../data/input";
const std::string g_output_path = "../../data/tmp/raw_input";

struct DocInfo{
  std::string title;
  std::string content;
  std::string url;
};

bool EnumFile(const std::string& input_path, 
    std::vector<std::string>* file_list){
  namespace fs = boost::filesystem;
  fs::path root_path(input_path);
  if(!fs::exists(root_path)) {
    std::cout << "input_path not exist! input_path="
              << input_path << std::endl;
    return false;
  }
  fs::recursive_directory_iterator end_iter;
  for(fs::recursive_directory_iterator iter(root_path); iter != end_iter; ++iter) {
    if(!fs::is_regular_file(*iter)) {
      continue;
    }
    if(iter->path().extension() != ".html") {
      continue;
    }
    file_list->push_back(iter->path().string());
  }
  return true;
}

bool ParseTitle(const std::string& html, std::string* title) {
  size_t beg = html.find("<title>");
  if(beg == std::string::npos) {
    std::cout << "<title> not found!" << std::endl;
    return false;
  }
  size_t end = html.find("</title>");
  if(end == std::string::npos) {
    std::cout << "</title> not found!" << std::endl;
    return false;
  }
  beg += std::string("<title>").size();
  if(beg > end) {
    std::cout << "beg end error!" << std::endl;
    return false;
  }
  *title = html.substr(beg, end - beg);
  return true;
}

bool ParseContent(const std::string& html, 
    std::string* content) {
  bool is_content = true;
  for (auto c : html) {
    if(is_content) {
      if(c == '<') {
        is_content = false;
      } else {
        if (c == '\n') {
          c = ' '; //此处把换行替换为空格，为了最终的行文本文件
        }
        content->push_back(c);
      }
    } else {
      if(c == '>') {
        is_content = true;
      }
    }
  }
  return true;
}

bool ParseUrl(const std::string& file_path,
    std::string* url) {
  std::string prefix = "http://www.boost.org/doc/libs/1_53_0/doc/";
  std::string tail = file_path.substr(g_input_path.size());
  *url = prefix + tail;
  return true;
}

bool ParseFile(const std::string& file_path, DocInfo* doc_info) {
  std::string html;
  bool ret = FileUtil::Read(file_path, &html);
  if(!ret) {
    std::cout << "Read file failed! file_path="
      << file_path << std::endl;
    return false;
  }
  ret = ParseTitle(html, &doc_info->title);
  if(!ret) {
    std::cout << "ParseTitle failed! file_path="
      << file_path << std::endl;
    return false;
  }
  ret = ParseContent(html, &doc_info->content);
  if(!ret) {
    std::cout << "ParseContent failed! file_path="
      << file_path << std::endl;
    return false;
  }
  ret = ParseUrl(file_path, &doc_info->url);
  if(!ret) {
    std::cout << "ParseUrl failed! file_path="
      << file_path << std::endl;
    return false;
    }
  return true;
}

void WriteOutput(const DocInfo& doc_info, std::ofstream& file) {
  std::string line = doc_info.title + "\3"
    + doc_info.url + "\3" + doc_info.content + "\n";
  file.write(line.c_str(), line.size());
}


int main(){
  std::vector<std::string> file_list;
  bool ret = EnumFile(g_input_path, &file_list);
  if(!ret){
    std::cout << "EnumFile failed!" << std::endl; 
    return 1;
  }

#if 0
  //TODO 验证EnumFile是不是正确
  for(const auto& file_path : file_list) {
    std::cout << file_path << std::endl;
  }
#endif

  std::ofstream output_file(g_output_path.c_str()); 
  if(!output_file.is_open()) {
    std::cout << "open output_file failed! g_output_path=" << g_output_path << std::endl;
    return 1;
  }
  for(const auto& file_path : file_list) {
    DocInfo info;
    ret = ParseFile(file_path, &info);
    if(!ret) {
      std::cout << "ParseFile failed! file_path=" << file_path << std::endl;
      // return 1;
      continue;
    }
  WriteOutput(info, output_file);
  }
  output_file.close();
  return 0;
}
