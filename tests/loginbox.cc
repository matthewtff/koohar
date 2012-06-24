#include <webui.hh>

#include "loginbox.hh"

using namespace koohar;
using namespace koohar::webui;

LoginBox::LoginBox(Request& NewReq, WebPage* NewPage) :
	m_req(NewReq), m_page(NewPage)
{
}

void LoginBox::get()
{
	if (!m_page)
		return;
	if (!m_req.session("user_name").empty()) {
		ObjectPtr welcome_par = m_page->factory<Object>("#welcome_par");
		welcome_par->set("text") = "Welcome, " + m_req.session("user_name");
		ObjectPtr login_form = m_page->factory<Object>("#login_form");
		login_form->set("style") += "display:none;";
	} else {
		ObjectPtr logout_form = m_page->factory<Object>("#logout_form");
		logout_form->set("style") += "display:none;";
	}
}

void LoginBox::post()
{
	if (!m_req.body("login_button").empty())
		authorise();
	if (!m_req.body("logout_button").empty())
		logout();
	get();
}

void LoginBox::authorise()
{
	if (!m_req.body("login").empty() && !m_req.body("password").compare("epicpassword"))
		m_req.session("user_name") = m_req.body("login");
}

void LoginBox::logout()
{
	m_req.unsetSession("user_name");
}

