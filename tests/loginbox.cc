#include <webui.hh>

#include "loginbox.hh"

using namespace koohar;
using namespace koohar::webui;

LoginBox::LoginBox(Request& NewReq, WebPage* NewPage) : _req(NewReq), _page(NewPage)
{
}

void LoginBox::get()
{
	if (!_page)
		return;
	if (!_req.session("user_name").empty()) {
		ObjectPtr welcome_par = _page->factory<Object>("#welcome_par");
		welcome_par->set("text") = "Welcome, " + _req.session("user_name");
		ObjectPtr login_form = _page->factory<Object>("#login_form");
		login_form->set("style") += "display:none;";
	} else {
		ObjectPtr logout_form = _page->factory<Object>("#logout_form");
		logout_form->set("style") += "display:none;";
	}
}

void LoginBox::post()
{
	if (!_req.body("login_button").empty())
		authorise();
	if (!_req.body("logout_button").empty())
		logout();
	get();
}

void LoginBox::authorise()
{
	if (!_req.body("login").empty() && !_req.body("password").compare("epicpassword"))
		_req.session("user_name") = _req.body("login");
}

void LoginBox::logout()
{
	_req.unsetSession("user_name");
}

