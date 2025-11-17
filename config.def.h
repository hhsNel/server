#define SLOW_LORIS_TIMEOUT 4 /* 4 secs without sending? you have been terminated */
#define FILE_SERVE_BUFF_SIZE 1024
#define BUFF_GRAN 1024
#define READ_MAX 256
#define MAX_HTTP_REQ_LINE_SIZE 4*1024
#define MAX_HTTP_HEADERS_SIZE 16*1024

#define STATS 1 /* whether to save statistics or not */
#define STATS_FILE "statistics/statistics-%s.txt"
#define STATS_WRITE_PERIOD 4 /* flush stats to file every 4 requests */

#define LOGS 1 /* whether to save detailed logs or not */
#define LOGS_FILE "logs/logs-%s.txt"
#define LOGS_WRITE_PERIOD 1 /* flush all logs every request IF they were changed */
#define XFF_LOG_ARR_GRAN 16
#define LOG_DUMP_IF(req, xff_header) \
		xff_log_exists(xff_header) || \
		is_sus(&(req))
#define IS_SUS_IF(req) \
		SUS_CHECK("path contains 'admin'", bp_contains_str((req).buff, (req).path, "admin")) \
		SUS_CHECK("path contains 'conf'", bp_contains_str((req).buff, (req).path, "conf")) \
		SUS_CHECK("possible path traversal", bp_contains_str((req).buff, (req).path, "..")) \
		SUS_CHECK("possible path traversal", bp_contains_str((req).buff, (req).path, "%2e")) \
		SUS_CHECK("possible path traversal", bp_contains_str((req).buff, (req).path, "%2E")) \
		SUS_CHECK("shell mentioned in path", bp_contains_str((req).buff, (req).path, "sh")) \
		SUS_CHECK("path contains 'curl'", bp_contains_str((req).buff, (req).path, "curl")) \
		SUS_CHECK("path contains 'wget'", bp_contains_str((req).buff, (req).path, "wget"))

#define USE_UNIX_SOCKET 0
#define UNIX_SOCK_PATH "serv.sock"
unsigned int port = 8080;

unsigned int backlog = 4;

ResolvFunc index_page[] = {
	{ serve_headers_html,      {} },
	{ serve_file,              {.str="files/index.html"} },
	{ NULL, {} },
};

ResolvFunc favicon[] = {
	{ serve_headers_jpg,       {} },
	{ serve_file,              {.str="images/favicon.jpg"} },
	{ NULL, {} },
};

ResolvFunc exec_neofetch[] = {
	{ serve_headers_plaintext, {} },
	{ serve_exec_shell,        {.str="neofetch --stdout"} },
	{ NULL, {} },
};

/* ResolvFunc pass_to_other_server[] = {
	{ pass_all_to_local_port,  {.i=40392} },
	{ NULL, {} },
}; */

ResolvFunc guessed_my_favorite_number[] = {
	{ serve_headers_html,      {} },
	{ serve_file,              {.str="files/correct.html"} },
	{ NULL, {} },
};

ResolvFunc incorrect[] = {
	{ serve_headers_html,      {} },
	{ serve_file,              {.str="files/incorrect.html"} },
	{ NULL, {} },
};

ResolvFunc form[] = {
	{ serve_headers_html,      {} },
	{ serve_file,              {.str="files/guess-number-form.html"} },
	{ NULL, {} },
};

ResolvFunc friends[] = {
	{ serve_headers_html,      {} },
	{ serve_file,              {.str="files/friends.html"} },
	{ NULL, {} },
};

ResolvFunc spo[] = {
	{ serve_headers_html,      {} },
	{ serve_file,              {.str="files/spo.html"} },
	{ NULL, {} },
};

ResolvFunc styling[] = {
	{ serve_headers_css,       {} },
	{ serve_plaintext,         {.str="body {\n\tbackground-color: #"} },
	{ serve_cookie_value,      {.str="clr-background"} },
	{ serve_plaintext,         {.str=";\n\tcolor: #"} },
	{ serve_cookie_value,      {.str="clr-foreground"} },
	{ serve_plaintext,         {.str=";\n}\n"} },
	{ serve_plaintext,         {.str="a:link {\n\tcolor: #"} },
	{ serve_cookie_value,      {.str="clr-unvisited"} },
	{ serve_plaintext,         {.str=";\n}\n"} },
	{ serve_plaintext,         {.str="a:visited {\n\tcolor: #"} },
	{ serve_cookie_value,      {.str="clr-visited"} },
	{ serve_plaintext,         {.str=";\n}\n"} },
	{ serve_plaintext,         {.str="input {\n\tbackground-color: #"} },
	{ serve_cookie_value,      {.str="clr-inputbg"} },
	{ serve_plaintext,         {.str=";\n\tcolor: #"} },
	{ serve_cookie_value,      {.str="clr-inputfg"} },
	{ serve_plaintext,         {.str=";\n}\n"} },
	{ NULL, {} },
};

ResolvFunc set_cookie_graphical[] = {
	{ serve_headers_html,      {} },
	{ serve_file,              {.str="files/set-cookie-graphical.html"} },
	{ NULL, {} },
};

ResolvFunc set_cookie_back[] = {
	{ serve_http_status,       {.str="301 Moved Permanently"} },
	{ serve_plaintext,         {.str="Set-Cookie: "} },
	{ serve_query_param,       {.str="name"} },
	{ serve_plaintext,         {.str="="} },
	{ serve_query_param,       {.str="value"} },
	{ serve_plaintext,         {.str="\r\nLocation: /\r\n\r\n"} },
	{ NULL, {} },
};

ResolvFunc set_cookie_default[] = {
	{ serve_file,              {.str="files/set-cookie-default.http"} },
	{ NULL, {} },
};

ResolvFunc characters[] = {
	{ serve_headers_html,      {} },
	{ serve_file,              {.str="files/characters.html"} },
	{ NULL, {} },
};

ResolvFunc proclamation_of_the_new_order_mp3[] = {
	{ serve_headers_mpeg,      {} },
	{ serve_file,              {.str="files/proclamation-of-the-new-order.mp3"} },
	{ NULL, {} },
};

ResolvFunc proclamation_of_the_new_order_ogg[] = {
	{ serve_headers_ogg,      {} },
	{ serve_file,              {.str="files/proclamation-of-the-new-order.ogg"} },
	{ NULL, {} },
};

ResolvFunc proclamation_of_the_new_order_mp3_full[] = {
	{ serve_headers_mpeg,      {} },
	{ serve_file,              {.str="files/proclamation-of-the-new-order-full.mp3"} },
	{ NULL, {} },
};

ResolvFunc proclamation_of_the_new_order_ogg_full[] = {
	{ serve_headers_ogg,      {} },
	{ serve_file,              {.str="files/proclamation-of-the-new-order-full.ogg"} },
	{ NULL, {} },
};

ResolvFunc init[] = { /* "entry point" */
	{ if_is_path_no_query,     {.jump=index_page,.str="/"} },
	{ if_is_path_no_query,     {.jump=favicon,.str="/favicon.ico"} },
	{ if_is_path_no_query,     {.jump=styling,.str="/styling.css"} },
	{ if_is_path_no_query,     {.jump=exec_neofetch,.str="/my-specs"} },
	/* { if_is_path,           {.jump=pass_to_other_server,.str="/other-website"} }, */
	{ if_is_path_no_query,     {.jump=form,.str="/my-form"} },
	{ if_is_path,     {.jump=guessed_my_favorite_number,.str="/form-response?number=16"} },
	{ if_path_begins,          {.jump=incorrect,.str="/form-response?number="} },
	{ if_is_path_no_query,     {.jump=friends,.str="/friends"} },
	{ if_is_path_no_query,     {.jump=spo,.str="/spo"} },
	{ if_is_path_no_query,     {.jump=set_cookie_graphical,.str="/set-cookie-graphical"} },
	{ if_is_path_no_query,     {.jump=set_cookie_back,.str="/set-cookie"} },
	{ if_is_path_no_query,     {.jump=set_cookie_default,.str="/default-cookies"} },
	{ if_is_path_no_query,     {.jump=characters,.str="/characters"} },
	{ if_is_path_no_query,     {.jump=proclamation_of_the_new_order_mp3,.str="/proclamation-of-the-new-order.mp3"} },
	{ if_is_path_no_query,     {.jump=proclamation_of_the_new_order_ogg,.str="/proclamation-of-the-new-order.ogg"} },
	{ if_is_path_no_query,     {.jump=proclamation_of_the_new_order_mp3_full,.str="/proclamation-of-the-new-order-full.mp3"} },
	{ if_is_path_no_query,     {.jump=proclamation_of_the_new_order_ogg_full,.str="/proclamation-of-the-new-order-full.ogg"} },
	{ NULL, {} },
};

