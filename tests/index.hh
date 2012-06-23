#include <server.hh>
#include <webpage.hh>
#include <webui.hh>

#include "loginbox.hh"

class IndexPage : public koohar::WebPage {
public:
	IndexPage (koohar::Request& Req, koohar::Response& Res, const std::string& FileName);
	~IndexPage () {}
	void load ();
	void get ();
	void post();
private:
	LoginBox _login_box;
}; // class IndexPage
