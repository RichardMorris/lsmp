/* Form definition file generated with fdesign. */

#include "forms.h"
#include "fd_panels.h"
#include "../forms/edit.h"
#include "../forms/fract.h"

FL_FORM *main_panel;

FL_OBJECT
        *deffile_input,
        *eqn_edit,
        *quit_button,
        *xmin_counter,
        *xmax_counter,
        *ymin_counter,
        *ymax_counter,
        *save_but,
        *load_but,
        *opt_button,
        *help_button,
        *load_but,
        *zmin_counter,
        *zmax_counter,
        *wmax_counter,
        *wmin_counter;

void create_form_main_panel(void)
{
  FL_OBJECT *obj;
  main_panel = fl_bgn_form(FL_NO_BOX,610.0,420.0);
  obj = fl_add_box(FL_UP_BOX,0.0,0.0,610.0,420.0,"");
    fl_set_object_lsize(obj,11.000000);
  deffile_input = obj = fl_add_input(FL_NORMAL_INPUT,40.0,320.0,410.0,30.0,"File");
    fl_set_object_lsize(obj,11.000000);
    fl_set_call_back(obj,filename_cb,1);
  eqn_edit = obj = fl_add_edit(FL_NORMAL_EDIT,20.0,10.0,570.0,70.0,"Equation");
    fl_set_object_lsize(obj,11.000000);
    fl_set_object_align(obj,FL_ALIGN_TOP);
    fl_set_call_back(obj,filename_cb,4);
  obj = fl_add_text(FL_NORMAL_TEXT,20.0,370.0,260.0,30.0,"Acurve4");
    fl_set_object_lcol(obj,4);
    fl_set_object_lsize(obj,20.000000);
  quit_button = obj = fl_add_button(FL_NORMAL_BUTTON,540.0,370.0,50.0,30.0,"Quit");
    fl_set_object_lsize(obj,11.000000);
    fl_set_call_back(obj,exit_cb,0);
  xmin_counter = obj = fl_add_fract(FL_NORMAL_FRACT,100.0,260.0,210.0,30.0,"x");
    fl_set_object_lsize(obj,11.000000);
    fl_set_object_align(obj,FL_ALIGN_LEFT);
    fl_set_call_back(obj,bound_cb,1);
  xmax_counter = obj = fl_add_fract(FL_NORMAL_FRACT,320.0,260.0,210.0,30.0,"");
    fl_set_object_lsize(obj,11.000000);
    fl_set_call_back(obj,bound_cb,2);
  ymin_counter = obj = fl_add_fract(FL_NORMAL_FRACT,100.0,210.0,210.0,30.0,"y");
    fl_set_object_lsize(obj,11.000000);
    fl_set_object_align(obj,FL_ALIGN_LEFT);
    fl_set_call_back(obj,bound_cb,3);
  ymax_counter = obj = fl_add_fract(FL_NORMAL_FRACT,320.0,210.0,210.0,30.0,"");
    fl_set_object_lsize(obj,11.000000);
    fl_set_call_back(obj,bound_cb,4);
  obj = fl_add_text(FL_NORMAL_TEXT,180.0,290.0,40.0,20.0,"Low");
    fl_set_object_lsize(obj,11.000000);
  obj = fl_add_text(FL_NORMAL_TEXT,400.0,290.0,50.0,20.0,"High");
    fl_set_object_lsize(obj,11.000000);
  save_but = obj = fl_add_button(FL_NORMAL_BUTTON,530.0,320.0,60.0,30.0,"Save");
    fl_set_object_lsize(obj,11.000000);
    fl_set_call_back(obj,filename_cb,3);
  load_but = obj = fl_add_button(FL_NORMAL_BUTTON,460.0,320.0,60.0,30.0,"Load");
    fl_set_object_lsize(obj,11.000000);
    fl_set_call_back(obj,filename_cb,2);
  opt_button = obj = fl_add_button(FL_NORMAL_BUTTON,410.0,370.0,60.0,30.0,"Options");
    fl_set_object_lsize(obj,11.000000);
    fl_set_call_back(obj,opt_cb,0);
  help_button = obj = fl_add_button(FL_NORMAL_BUTTON,480.0,370.0,50.0,30.0,"Help");
    fl_set_object_lsize(obj,11.000000);
    fl_set_call_back(obj,help_cb,0);
  load_but = obj = fl_add_button(FL_NORMAL_BUTTON,340.0,370.0,60.0,30.0,"RUN");
    fl_set_object_lsize(obj,16.000000);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_call_back(obj,run_cb,0);
  zmin_counter = obj = fl_add_fract(FL_NORMAL_FRACT,100.0,160.0,210.0,30.0,"z");
    fl_set_object_lsize(obj,11.000000);
    fl_set_object_align(obj,FL_ALIGN_LEFT);
    fl_set_call_back(obj,bound_cb,5);
  zmax_counter = obj = fl_add_fract(FL_NORMAL_FRACT,320.0,160.0,210.0,30.0,"");
    fl_set_object_lsize(obj,11.000000);
    fl_set_call_back(obj,bound_cb,6);
  wmax_counter = obj = fl_add_fract(FL_NORMAL_FRACT,320.0,110.0,210.0,30.0,"");
    fl_set_object_lsize(obj,11.000000);
    fl_set_call_back(obj,bound_cb,8);
  wmin_counter = obj = fl_add_fract(FL_NORMAL_FRACT,100.0,110.0,210.0,30.0,"w");
    fl_set_object_lsize(obj,11.000000);
    fl_set_object_align(obj,FL_ALIGN_LEFT);
    fl_set_call_back(obj,bound_cb,7);
  fl_end_form();
}

/*---------------------------------------*/

FL_FORM *ac4_help;

FL_OBJECT
        *help_browser,
        *help_done;

void create_form_ac4_help(void)
{
  FL_OBJECT *obj;
  ac4_help = fl_bgn_form(FL_NO_BOX,610.0,460.0);
  obj = fl_add_box(FL_UP_BOX,0.0,0.0,610.0,460.0,"");
    fl_set_object_lsize(obj,11.000000);
  help_browser = obj = fl_add_browser(FL_NORMAL_BROWSER,30.0,20.0,540.0,370.0,"");
    fl_set_object_lsize(obj,11.000000);
  obj = fl_add_text(FL_NORMAL_TEXT,110.0,410.0,260.0,30.0,"Help");
    fl_set_object_lsize(obj,20.000000);
  help_done = obj = fl_add_button(FL_NORMAL_BUTTON,470.0,410.0,90.0,30.0,"Done");
    fl_set_object_lsize(obj,11.000000);
    fl_set_call_back(obj,help_cb,2);
  fl_end_form();
}

/*---------------------------------------*/

FL_FORM *option_form;

FL_OBJECT
        *dp_input,
        *opt_done,
        *coarse_input,
        *fine_input,
        *boxes_input,
        *edges_input,
        *add_but,
        *replace_but,
        *opt_done;

void create_form_option_form(void)
{
  FL_OBJECT *obj;
  option_form = fl_bgn_form(FL_NO_BOX,200.0,330.0);
  obj = fl_add_box(FL_UP_BOX,0.0,0.0,200.0,330.0,"");
    fl_set_object_lsize(obj,11.000000);
  dp_input = obj = fl_add_input(FL_INT_INPUT,10.0,290.0,40.0,30.0,"Precision");
    fl_set_object_lsize(obj,11.000000);
    fl_set_object_align(obj,FL_ALIGN_RIGHT);
    fl_set_call_back(obj,opt_cb,1);
  opt_done = obj = fl_add_button(FL_NORMAL_BUTTON,140.0,290.0,50.0,30.0,"Done");
    fl_set_object_lsize(obj,11.000000);
    fl_set_call_back(obj,opt_cb,-1);
  obj = fl_add_text(FL_NORMAL_TEXT,50.0,160.0,90.0,30.0,"Resolutions");
    fl_set_object_lsize(obj,11.000000);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  coarse_input = obj = fl_add_input(FL_INT_INPUT,10.0,130.0,90.0,30.0,"Coarse");
    fl_set_object_lsize(obj,11.000000);
    fl_set_object_align(obj,FL_ALIGN_RIGHT);
    fl_set_call_back(obj,opt_cb,4);
  fine_input = obj = fl_add_input(FL_INT_INPUT,10.0,90.0,90.0,30.0,"Fine");
    fl_set_object_lsize(obj,11.000000);
    fl_set_object_align(obj,FL_ALIGN_RIGHT);
    fl_set_call_back(obj,opt_cb,5);
  boxes_input = obj = fl_add_input(FL_INT_INPUT,10.0,50.0,90.0,30.0,"Boxes");
    fl_set_object_lsize(obj,11.000000);
    fl_set_object_align(obj,FL_ALIGN_RIGHT);
    fl_set_call_back(obj,opt_cb,6);
  edges_input = obj = fl_add_input(FL_INT_INPUT,10.0,10.0,90.0,30.0,"edges");
    fl_set_object_lsize(obj,11.000000);
    fl_set_object_align(obj,FL_ALIGN_RIGHT);
    fl_set_call_back(obj,opt_cb,7);
  add_but = obj = fl_add_lightbutton(FL_RADIO_BUTTON,10.0,230.0,80.0,30.0,"Add");
    fl_set_object_lsize(obj,11.000000);
    fl_set_call_back(obj,opt_cb,2);
  replace_but = obj = fl_add_lightbutton(FL_RADIO_BUTTON,110.0,230.0,80.0,30.0,"Replace");
    fl_set_object_lsize(obj,11.000000);
    fl_set_call_back(obj,opt_cb,3);
  obj = fl_add_text(FL_NORMAL_TEXT,50.0,260.0,90.0,30.0,"Write Mode");
    fl_set_object_lsize(obj,11.000000);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  opt_done = obj = fl_add_button(FL_NORMAL_BUTTON,80.0,195.0,50.0,30.0,"New");
    fl_set_object_lsize(obj,11.000000);
    fl_set_call_back(obj,opt_cb,10);
  fl_end_form();
}

/*---------------------------------------*/

FL_FORM *run_form;

FL_OBJECT
        *abort_but,
        *report_brow,
        *flush_but;

void create_form_run_form(void)
{
  FL_OBJECT *obj;
  run_form = fl_bgn_form(FL_NO_BOX,410.0,130.0);
  obj = fl_add_box(FL_UP_BOX,0.0,0.0,410.0,130.0,"");
    fl_set_object_lsize(obj,11.000000);
  abort_but = obj = fl_add_button(FL_NORMAL_BUTTON,280.0,90.0,110.0,30.0,"Abort");
    fl_set_object_lsize(obj,11.000000);
  obj = fl_add_text(FL_NORMAL_TEXT,30.0,90.0,190.0,30.0,"Running");
    fl_set_object_lsize(obj,11.000000);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  report_brow = obj = fl_add_browser(FL_NORMAL_BROWSER,10.0,20.0,380.0,60.0,"");
    fl_set_object_lsize(obj,11.000000);
  flush_but = obj = fl_add_button(FL_NORMAL_BUTTON,160.0,90.0,110.0,30.0,"Show");
    fl_set_object_lsize(obj,11.000000);
  fl_end_form();
}

/*---------------------------------------*/

void create_the_forms(void)
{
  create_form_main_panel();
  create_form_ac4_help();
  create_form_option_form();
  create_form_run_form();
}

