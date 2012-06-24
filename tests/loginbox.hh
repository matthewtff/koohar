#include <webpage.hh>
#include <request.hh>

class LoginBox {
public:
	LoginBox(koohar::Request& NewReq, koohar::WebPage* NewPage = 0);
	void get();
	void post();

private:
	void authorise();
	void logout();

private:
	koohar::Request& m_req;
	koohar::WebPage* m_page;

}; // class LoginBox

