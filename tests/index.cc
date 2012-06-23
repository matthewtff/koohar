#include "index.hh"

#include <iostream>

using namespace koohar;
using namespace koohar::webui;
using namespace std;

IndexPage::IndexPage (koohar::Request& Req, koohar::Response& Res, const std::string& FileName) : WebPage(Req, Res, FileName),
	_login_box(Req, this)
{
	load();
}

void IndexPage::load()
{
	if (!req().method().compare("POST"))
		post();
	else
		get();
}

void IndexPage::get()
{
	_login_box.get();
}

void IndexPage::post()
{
	_login_box.post();
}

