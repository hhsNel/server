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

ResolvFunc init[] = { /* "entry point" */
	{ if_is_path,              {.jump=index_page,.str="/"} },
	{ if_is_path,              {.jump=favicon,.str="/favicon.ico"} },
	{ if_is_path,              {.jump=exec_neofetch,.str="/my-specs"} },
	/* { if_is_path,              {.jump=pass_to_other_server,.str="/other-website"} }, */
	{ if_is_path,              {.jump=form,.str="/my-form"} },
	{ if_is_path,              {.jump=guessed_my_favorite_number,.str="/form-response?number=16"} },
	{ if_path_begins,          {.jump=incorrect,.str="/form-response?number="} },
	{ NULL, {} },
};

