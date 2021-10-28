#pragma once
#include <string>

namespace vtw
{
namespace file
{

/**
* 判断文件或目录是否存在
*/
bool Exists(const std::string &path);

/*
* 递归创建目录
* 如果创建失败，中途已创建的目录不会删除
*/
bool CreateDirectory(const std::string &path);

/**
* 获取上一级目录
* 如果没有则返回空串
* c:/foo/bar/123.txt --> c:/foo/bar/
* c:/foo/bar/ --> c:/foo/
* c:/ --> ""
*/
std::string GetParentDirectory(const std::string &path);

/**
* 删除指定文件
*/
bool DeleteFile(const std::string &filename);

} // namespace file
} // namespace vtw
