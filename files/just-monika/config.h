#define RF_AUDIO(name, filename) \
	ResolvFunc name[] = { \
		{ serve_http_status,        {.str="200 OK"} }, \
		{ serve_plaintext,          {.str="Content-Type: audio/ogg\r\n"} }, \
		{ serve_file_content_length,{.str="files/just-monika/" filename} }, \
		{ serve_plaintext,          {.str="\r\n"} }, \
		{ serve_file,               {.str="files/just-monika/" filename} }, \
		{ NULL,                     {} }, \
	};

RF_AUDIO(just_monika_music_daijoubu, "music/daijoubu.ogg")
RF_AUDIO(just_monika_music_doki_doki_literature_club, "music/doki-doki-literature-club.ogg")
RF_AUDIO(just_monika_music_dreams_of_love_and_literature, "music/dreams-of-love-and-literature.ogg")
RF_AUDIO(just_monika_music_i_still_love_you, "music/i-still-love-you.ogg")
RF_AUDIO(just_monika_music_just_monika, "music/just-monika.ogg")
RF_AUDIO(just_monika_music_my_confession, "music/my-confession.ogg")
RF_AUDIO(just_monika_music_my_feelings, "music/my-feelings.ogg")
RF_AUDIO(just_monika_music_ohayu_sayori, "music/ohayu-sayori.ogg")
RF_AUDIO(just_monika_music_ok_everyone, "music/ok-everyone.ogg")
RF_AUDIO(just_monika_music_play_with_me, "music/play-with-me.ogg")
RF_AUDIO(just_monika_music_poem_panic, "music/poem-panic.ogg")
RF_AUDIO(just_monika_music_sayo_nara, "music/sayo-nara.ogg")
RF_AUDIO(just_monika_music_your_reality, "music/your-reality.ogg")

ResolvFunc just_monika_index[] = {
	{ serve_headers_html,           {} },
	{ serve_file,                   {.str="files/just-monika/index.html"} },
	{ NULL,                         {} },
};

ResolvFunc just_monika[] = {
	{ if_is_path_no_query,          {.jump=just_monika_music_daijoubu, .str="/just-monika/music/daijoubu"} },
	{ if_is_path_no_query,          {.jump=just_monika_music_doki_doki_literature_club, .str="/just-monika/music/doki-doki-literature-club"} },
	{ if_is_path_no_query,          {.jump=just_monika_music_dreams_of_love_and_literature, .str="/just-monika/music/dreams-of-love-and-literature"} },
	{ if_is_path_no_query,          {.jump=just_monika_music_i_still_love_you, .str="/just-monika/music/i-still-love-you"} },
	{ if_is_path_no_query,          {.jump=just_monika_music_just_monika, .str="/just-monika/music/just-monika"} },
	{ if_is_path_no_query,          {.jump=just_monika_music_my_confession, .str="/just-monika/music/my-confession"} },
	{ if_is_path_no_query,          {.jump=just_monika_music_my_feelings, .str="/just-monika/music/my-feelings"} },
	{ if_is_path_no_query,          {.jump=just_monika_music_ohayu_sayori, .str="/just-monika/music/ohayu-sayori"} },
	{ if_is_path_no_query,          {.jump=just_monika_music_ok_everyone, .str="/just-monika/music/ok-everyone"} },
	{ if_is_path_no_query,          {.jump=just_monika_music_play_with_me, .str="/just-monika/music/play-with-me"} },
	{ if_is_path_no_query,          {.jump=just_monika_music_poem_panic, .str="/just-monika/music/poem-panic"} },
	{ if_is_path_no_query,          {.jump=just_monika_music_sayo_nara, .str="/just-monika/music/sayo-nara"} },
	{ if_is_path_no_query,          {.jump=just_monika_music_your_reality, .str="/just-monika/music/your-reality"} },
	{ if_is_path_no_query,          {.jump=just_monika_index, .str="/just-monika"} },
	{ NULL,                         {} },
};

