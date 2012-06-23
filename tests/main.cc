#include <iostream>
#include <stdlib.h>
#include <string>

#include <app.hh>
#include <webpage.hh>

#include "win.hh"
#include "index.hh"

static std::string views_dir("views/");

HandlerRet sigHandler (HandlerGet sig)
{
	if (sig == CTRL_C_EVENT) {
		std::cout << "\texiting...\n";
		exit(0);
	}
}

void ServerLogic(koohar::Request& Req, koohar::Response &Res, void*)
{
	if (Req.contains("/crimson")) {
		koohar::WebPage crimson_page(Req, Res, views_dir + "crimson.html"); // use of standart WebPage
		crimson_page.render();
	} else if (Req.contains("/monge")) {
		koohar::WebPage monge_page(Req, Res, views_dir + "monge.html");
		monge_page.render();
	} else if (Req.contains("/")) {
		IndexPage index_page(Req, Res, views_dir + "./index.html");
		index_page.render();
	} else {
		Res.writeHead(404);
		Res.end();
	}
}

int main(int argc, char* argv[])
{
	setlocale(LC_ALL, "Russian");
	setHandler(CTRL_C_EVENT, sigHandler);
	std::string host = "localhost";
	unsigned short port = 7000;
	if (argc > 1)
		host.assign(argv[1]);
	if (argc > 2)
		port = static_cast<unsigned short>(atoi(argv[2]));
	koohar::App app(host, port, 1);
	app.config("static_dir", "public");
	app.config("static", "/html");
	app.config("static", "/music");
	app.config("static", "/images");
	app.config("static", "/stylesheets");
	app.config("static", "/js");
	app.listen(ServerLogic, 0);
	return 0;
}
