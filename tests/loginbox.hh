#include <webpage.hh>
#include <request.hh>

class LoginBox {
public:
	LoginBox(koohar::Request& NewReq, koohar::WebPage* NewPage = 0);
	void get();
	void post();
private:
	koohar::Request& _req;
	koohar::WebPage* _page;

	void authorise();
	void logout();
}; // class LoginBox

