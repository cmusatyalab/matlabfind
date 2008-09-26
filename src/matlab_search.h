/*
 *  MATLABFind
 *  A Diamond application for interoperating with MATLAB
 *  Version 1
 *
 *  Copyright (c) 2008 Carnegie Mellon University
 *  All Rights Reserved.
 *
 *  This software is distributed under the terms of the Eclipse Public
 *  License, Version 1.0 which can be found in the file named LICENSE.
 *  ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS SOFTWARE CONSTITUTES
 *  RECIPIENT'S ACCEPTANCE OF THIS AGREEMENT
 */

#ifndef	_MATLAB_SEARCH_H_
#define	_MATLAB_SEARCH_H_	1

#include <gtk/gtk.h>
#include "img_search.h"

class matlab_search: public img_search {
public:
	matlab_search(const char *name, char *descr);
	~matlab_search(void);

	void	save_edits();
	void	edit_search();
	void 	write_fspec(FILE* stream);
	void	write_config(FILE* stream, const char *data_dir);
	virtual	int	handle_config(int num_conf, char **datav);
	void	close_edit_win();
	bool	is_editable();
	virtual void 	region_match(RGBImage *img, bbox_list_t *blist);

private:
	char *		eval_function;
	char *		init_function;
	char *          threshold;
	char *          source_folder;
	GtkWidget       *eval_function_entry;
	GtkWidget       *init_function_entry;
	GtkWidget       *threshold_entry;
	GtkWidget	*source_folder_button;

	GtkWidget       *edit_window;
};


class matlab_factory: public img_factory {
public:
	matlab_factory() {
		set_name("MATLAB");
		set_description("matlab_search");
	}
	img_search *create(const char *name) {
		return new matlab_search(name, "MATLAB");
	}
	int is_example() {
		return(0);
	}
};

class matlab_codec_factory: public img_factory {
public:
	matlab_codec_factory() {
		set_name("MATLAB");
		set_description("RGB");
	}
	img_search *create(const char *name) {
		return new matlab_search(name, "RGB");
	}
	int is_example() {
		return(0);
	}
};


#ifdef __cplusplus
extern "C"
{
#endif


#ifdef __cplusplus
}
#endif

#endif	/* !_MATLAB_SEARCH_H_ */
