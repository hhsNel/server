unsigned int port = 8080;
unsigned int backlog = 4;

ResolvFunc index[] = {
	{ serve_headers_html,      {} },
	{ serve_file,              {.str:"files/index.html"} },
	NULL,
};

ResolvFunc favicon[] = {
	{ serve_headers_ico,       {} },
	{ serve_file,              {.str:"images/favicon.ico"} },
	NULL,
};

ResolvFunc exec_neofetch[] = {
	{ serve_headers_plaintext, {} },
	{ serve_exec_shell,        {.str:"neofetch"} },
	NULL,
};

ResolvFunc pass_to_other_server[] = {
	{ pass_all_to_local_port,  {.i:40392} },
	NULL,
};

ResolvFunc guessed_my_favorite_number[] = {
	{ serve_headers_html,      {} },
	{ serve_file,              {.str:"files/correct.html"} },
	NULL,
};

ResolvFunc incorrect[] = {
	{ serve_headers_html,      {} },
	{ serve_file,              {.str:"files/incorrect.html"} },
	NULL,
};

ResolvFunv form[] = {
	{ serve_headers_html,      {} },
	{ serve_file,              {.str:"files/guess-number-form.html"} },
	NULL,
};

ResolvFunc http_chain[] = {
	{ if_is_path,              {.jump:&index,.str:"/"} },
	{ if_is_path,              {.jump:&favicon,.str:"/favicon.ico"} },
	{ if_is_path,              {.jump:&about,.str:"/about.html"} },
	{ if_is_path,              {.jump:&exec_neofetch,.str:"/my-specs"} },
	{ if_is_path,              {.jump:&pass_to_other_server,.str:"/other-website"} },
	{ if_is_path,              {.jump:&form,.str:"/my-form"} },
	{ if_is_path,              {.jump:&guessed_my_favorite_number,.str:"/form-response?number=16"} },
	{ if_path_begins,          {.jump:&incorrect,.str:"/form-response?number="} },
	NULL,
};

ResolvFunc init[] = { /* "entry point" */
	{ if_is_http,              {.jump:&http_chain} },
	NULL,
};
