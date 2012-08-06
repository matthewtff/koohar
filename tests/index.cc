#include "index.hh"

#include <iostream>

using namespace koohar;
using namespace koohar::webui;
using namespace std;

IndexPage::IndexPage (koohar::Request& Req, koohar::Response& Res,
	const std::string& FileName) : WebPage(Req, Res, FileName),
	m_login_box(Req, this)
{
	load();
}

void IndexPage::load()
{
	if (!req().method() == Request::Post)
		post();
	else
		get();
}

void IndexPage::get()
{
	m_login_box.get();
}

void IndexPage::post()
{
	m_login_box.post();
}

