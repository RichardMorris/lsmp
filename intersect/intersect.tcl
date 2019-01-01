#!/usr/local/bin/wish -f

#set l [get_env]
#set lsmp_tcl [lindex $l 0]
set lsmp_tcl "/c/rjm/ntcl"
set auto_path "$lsmp_tcl $auto_path"
#fileselect_add_env [lindex $l 1]
source $lsmp_tcl/intersect.help
source $lsmp_tcl/intersect.syntax

set op_mode 0

# read in the options

if { $tk_version >= 4.0 } {
option readfile $lsmp_tcl/lsmp.Xdefaults
}

# Called when the tcl variables need updating 

proc update_variables {} {
	global frvals precision tolerance itterations colour
	global op_mode

	set l [get_options]
	set precision [lindex $l 0]
	set tolerance [format "%.1e" [lindex $l 1]]
	set itterations [lindex $l 2]
	set colour [lindex $l 3]
	change_precision
	set_op_mode [get_mode]
}

proc send_options {} {
	global precision tolerance itterations colour 
        global op_mode

        set_mode $op_mode
	set_options $precision $tolerance $itterations $colour 
}

proc change_precision {} {
        set_param_precision
}

# load a definition file

proc load_tcl {filename} {
	global temp_edit_file
	global current_file prog_name
        global tk_version

if { $tk_version >= 4.0 } {
        .menu.file entryconfigure "Save" -state normal
} else {
        .menu.file enable "Save" 
}
	load_cb $filename $temp_edit_file
	loadEditor $temp_edit_file
	update_parameters
	update_variables
	set current_file $filename
	set_titles
}

proc save_tcl {filename} {
	global temp_edit_file
	global current_file prog_name
        global tk_version

if { $tk_version >= 4.0 } {
        .menu.file entryconfigure "Save" -state normal
} else {
        .menu.file enable "Save" 
}
	send_parameters
	send_options
	save_cb $filename $temp_edit_file
	set current_file $filename
	set_titles
}

proc leaveEditor {} {
	global temp_edit_file

	saveEditor $temp_edit_file
	send_parameters
	send_options
	update_cb $temp_edit_file
	update_parameters
	update_variables
}

proc set_titles {} {
        global current_file prog_name

        wm title . [concat $prog_name "-" $current_file]
	wm title .shrink $current_file
        wm title .editor [concat "Edit -" $current_file]
        set_param_title $current_file
}

proc load_sphere {} {
	global temp_edit_file
	global current_file prog_name

	load_sphere_cb $temp_edit_file
	loadEditor $temp_edit_file
	update_parameters
	update_variables
	set current_file "Sphere"
	wm title . [concat $prog_name "-" $current_file]
}

proc load_plane {} {
	global temp_edit_file
	global current_file prog_name

	load_plane_cb $temp_edit_file
	loadEditor $temp_edit_file
	update_parameters
	update_variables
	set current_file "Plane"
	wm title . [concat $prog_name "-" $current_file]
}

proc load_box {} {
	global temp_edit_file
	global current_file prog_name

	load_box_cb $temp_edit_file
	loadEditor $temp_edit_file
	update_parameters
	update_variables
	set current_file "Box"
	wm title . [concat $prog_name "-" $current_file]
}

# runs the program

proc run_tcl {} {
	global frvals
	global temp_edit_file

	.r1.run configure -relief "sunken"
	.shrink.run configure -relief "sunken"
	update
	send_parameters
	send_options
#	set_variables $frvals(.r2.c1.x) $frvals(.r2.c2.x) $frvals(.r2.c3.x) \
#		$frvals(.r2.c1.y) $frvals(.r2.c2.y) $frvals(.r2.c3.y) 

	run_cb $temp_edit_file
	.r1.run configure -relief "raised"
	.shrink.run configure -relief "raised"
	update
}

proc run_plus_tcl {} {
	global frvals
	global temp_edit_file

	.r1.runp configure -relief "sunken"
	.shrink.runp configure -relief "sunken"
	update
	send_parameters
	send_options
#	set_variables $frvals(.r2.c1.x) $frvals(.r2.c2.x) $frvals(.r2.c3.x) \
#		$frvals(.r2.c1.y) $frvals(.r2.c2.y) $frvals(.r2.c3.y) 

	run_plus_cb $temp_edit_file
	.r1.runp configure -relief "raised"
	.shrink.runp configure -relief "raised"
	update
}

proc run_minus_tcl {} {
	global frvals temp_edit_file

	.r1.runm configure -relief "sunken"
	.shrink.runm configure -relief "sunken"
	update
	send_parameters
	send_options
#	set_variables $frvals(.r2.c1.x) $frvals(.r2.c2.x) $frvals(.r2.c3.x) \
#		$frvals(.r2.c1.y) $frvals(.r2.c2.y) $frvals(.r2.c3.y) 

	run_minus_cb $temp_edit_file
	.r1.runm configure -relief "raised"
	.shrink.runm configure -relief "raised"
	update
}

#
# called whenever parameters have been changed

proc param_changed {} {
	send_parameters
	send_options
	draw_special_cb
}

# This proc is called whenever the targets popup menu is posted
# it reads the names from geomview creating one menu item for each

proc post_targets {} {
        global targetname

        set l [get_targets_cb]
        set len [llength $l]

        .r1.mb.m delete 0 last

        .r1.mb.m add command -label "targetgeom" -command "set_target targetgeom"
        .r1.mb.m add separator
        for {set i 0} {$i < $len} {incr i} {
                .r1.mb.m add command -label [lindex $l $i] \
                        -command "set_target [lindex $l $i]"
        }
}

proc set_target {name} {
        global targetname
        target_cb $name
        set targetname $name
}


proc post_psurftargets {} {
        global psurftargetname

        set l [get_targets_cb]
        set len [llength $l]

        .r2.mb2.m delete 0 last

        .r2.mb2.m add command -label "targetgeom" -command "set_psurftarget targetgeom"
        .r2.mb2.m add separator
        for {set i 0} {$i < $len} {incr i} {
                .r2.mb2.m add command -label [lindex $l $i] \
                        -command "set_psurftarget [lindex $l $i]"
        }
}

proc set_psurftarget {name} {
        global psurftargetname
        psurf_target_cb $name
        set psurftargetname $name
}

proc post_pcurvetargets {} {
        global pcurvetargetname

        set l [get_targets_cb]
        set len [llength $l]

        .r2.mb5.m delete 0 last

        .r2.mb5.m add command -label "targetgeom" -command "set_pcurvetarget targetgeom"
        .r2.mb5.m add separator
        for {set i 0} {$i < $len} {incr i} {
                .r2.mb5.m add command -label [lindex $l $i] \
                        -command "set_pcurvetarget [lindex $l $i]"
        }
}

proc set_pcurvetarget {name} {
        global pcurvetargetname
        pcurve_target_cb $name
        set pcurvetargetname $name
}


proc post_impsurftargets {} {
        global impsurftargetname

        set l [get_targets_cb]
        set len [llength $l]

        .r2.mb4.m delete 0 last

        .r2.mb4.m add command -label "targetgeom" -command "set_impsurftarget targetgeom"
        .r2.mb4.m add separator
        for {set i 0} {$i < $len} {incr i} {
                .r2.mb4.m add command -label [lindex $l $i] \
                        -command "set_impsurftarget [lindex $l $i]"
        }
}

proc set_impsurftarget {name} {
        global impsurftargetname
        impsurf_target_cb $name
        set impsurftargetname $name
}

proc post_icurvetargets {} {
        global icurvetargetname

        set l [get_targets_cb]
        set len [llength $l]

        .r2.mb3.m delete 0 last

        .r2.mb3.m add command -label "targetgeom" -command "set_icurvetarget targetgeom"
        .r2.mb3.m add separator
        for {set i 0} {$i < $len} {incr i} {
                .r2.mb3.m add command -label [lindex $l $i] \
                        -command "set_icurvetarget [lindex $l $i]"
        }
}

proc set_icurvetarget {name} {
        global icurvetargetname
        icurve_target_cb $name
        set icurvetargetname $name
}

# The names are those to appear in menu
# the vals are those used to communicat with the C code
# the index refers to the position of the menu item
# which are radio buttons

set op_mode_array { "Simple" "Op: psurf" "Op: pcurve" "Op: impsurf" "Op: icurve+psurf" 
	"Op: psurf + Proj" "Op: icurve+psurf +Proj" }
set op_mode_vals { 0 1 3 8 -1 -2 -3 }
set op_mode_index 0

# this is called after a new mode has been chosen
# type is val of c variable 

proc set_op_mode {type} {
        global op_mode op_mode_name op_mode_array op_mode_vals op_mode_index

	set trim [string trimright $type]
	set op_mode $trim
	for {set i 0} {$i < [llength $op_mode_vals]} {incr i} {
		if { $trim == [lindex $op_mode_vals $i ] } {
        		set op_mode_name [lindex $op_mode_array $i]
			set op_mode_index $i
		} else {
		}
	}
	# now we want to change what is displayed
	# first test which fields are displayed

	set f0 [catch {pack info .r2} i]
	set f1 [catch {pack info .r2.l2} i]
	set f2 [catch {pack info .r2.mb2} i]
	set f3 [catch {pack info .r2.l3} i]
	set f4 [catch {pack info .r2.mb3} i]
	set f5 [catch {pack info .r2.l4} i]
	set f6 [catch {pack info .r2.mb4} i]
	set f7 [catch {pack info .r2.l5} i]
	set f8 [catch {pack info .r2.mb5} i]

	switch $op_mode_index {
	0 {
		if { $f1 == 0 } {pack forget .r2.l2} 
		if { $f2 == 0 } {pack forget .r2.mb2} 
		if { $f3 == 0 } {pack forget .r2.l3} 
		if { $f4 == 0 } {pack forget .r2.mb3} 
		if { $f5 == 0 } {pack forget .r2.l4} 
		if { $f6 == 0 } {pack forget .r2.mb4} 
		if { $f7 == 0 } {pack forget .r2.l5} 
		if { $f8 == 0 } {pack forget .r2.mb5} 
		if { $f0 == 0 } {pack forget .r2} 
	  }
	5 -
	1 {	# psurf
		if { $f0 != 0 } {pack .r2 -side top -fill x} 
		if { $f4 == 0 } {pack forget .r2.mb3} 
		if { $f1 != 0 } {pack .r2.l2 -side left -padx 1m -pady 2m} 
		if { $f2 != 0 } {pack .r2.mb2 -side left -padx 1m -pady 2m} 
		if { $f5 == 0 } {pack forget .r2.l4} 
		if { $f6 == 0 } {pack forget .r2.mb4} 
		if { $f7 == 0 } {pack forget .r2.l5} 
		if { $f8 == 0 } {pack forget .r2.mb5} 
		if { $f3 == 0 } {pack forget .r2.l3} 
	  }	
	2 {	# pcurve
		if { $f0 != 0 } {pack .r2 -side top -fill x} 
		if { $f1 == 0 } {pack forget .r2.l2} 
		if { $f2 == 0 } {pack forget .r2.mb2} 
		if { $f3 == 0 } {pack forget .r2.l3} 
		if { $f4 == 0 } {pack forget .r2.mb3} 
		if { $f7 != 0 } {pack .r2.l5 -side left -padx 1m -pady 2m} 
		if { $f8 != 0 } {pack .r2.mb5 -side left -padx 1m -pady 2m} 
		if { $f5 == 0 } {pack forget .r2.l4} 
		if { $f6 == 0 } {pack forget .r2.mb4} 
	  }
	3 {	#impsurf
		if { $f0 != 0 } {pack .r2 -side top -fill x} 
		if { $f1 == 0 } {pack forget .r2.l2} 
		if { $f2 == 0 } {pack forget .r2.mb2} 
		if { $f3 == 0 } {pack forget .r2.l3} 
		if { $f4 == 0 } {pack forget .r2.mb3} 
		if { $f7 == 0 } {pack forget .r2.l5} 
		if { $f8 == 0 } {pack forget .r2.mb5} 
		if { $f5 != 0 } {pack .r2.l4 -side left -padx 1m -pady 2m} 
		if { $f6 != 0 } {pack .r2.mb4 -side left -padx 1m -pady 2m} 
	  }
	6 -	
	4 {	#psurf-icurve
		if { $f0 != 0 } {pack .r2 -side top -fill x} 
		if { $f1 != 0 } {pack .r2.l2 -side left -padx 1m -pady 2m} 
		if { $f2 != 0 } {pack .r2.mb2 -side left -padx 1m -pady 2m} 
		if { $f3 != 0 } {pack .r2.l3 -side left -padx 1m -pady 2m} 
		if { $f4 != 0 } {pack .r2.mb3 -side left -padx 1m -pady 2m} 
		if { $f5 == 0 } {pack forget .r2.l4} 
		if { $f6 == 0 } {pack forget .r2.mb4} 
		if { $f7 == 0 } {pack forget .r2.l5} 
		if { $f8 == 0 } {pack forget .r2.mb5} 
	  }	
	}
		
#pack .r2.l2 .r2.mb2 -side left -padx 1m -pady 2m
#pack .r2.l3 .r2.mb3 -side left -padx 1m -pady 2m
}

# this command is called via the menu

proc op_mode_changed {} {
	global op_mode_vals op_mode_index

        set_op_mode [lindex $op_mode_vals $op_mode_index]
        set_mode [lindex $op_mode_vals $op_mode_index]
}

###################### Main Program #############################

#frame .menu -relief raised -borderwidth 1
#pack .menu -side top -fill x

menu .menu -tearoff 0 -takefocus 1  -takefocus 1

# File Menu: New/Load/Save/Save As/Quit

menu .menu.file -takefocus 0
.menu add cascade -label "File" -menu .menu.file -underline 0 
#menubutton .menu.file -text "File" -menu .menu.file -underline 0  -borderwidth 2
#menu .menu.file
#.menu.file add command -label "New" -command "puts stderr New" -underline 0
.menu.file add command -label "Load" -command "fileload load_tcl" -underline 0
.menu.file add command -label "Save" -command {save_tcl $current_file} -underline 0 -state disabled
.menu.file add command -label "Save As" -command "filesave save_tcl" -underline 5
.menu.file add separator
.menu.file add command -label "Shrink" -underline 5 -command { 
set i [wm geometry .]
set dotgeom [split $i {+ - x}]
if { [catch {set shrinkgeom} j ] } {
wm geometry .shrink [join [list + [lindex $dotgeom 2] + [lindex $dotgeom 3] ] {} ]
}
wm withdraw .
wm deiconify .shrink }
.menu.file add command -label "Quit" -command "destroy ." -underline 0

# Object Menu: New Object/Add/Replace/Animate/Target/Automatic

menu .menu.object -takefocus 0
.menu add cascade -label "Object" -menu .menu.object -underline 0 
#menubutton .menu.object -text "Object" -menu .menu.object -underline 0
#menu .menu.object
.menu.object add command -label "New Object" -command "object_cb New"\
	-underline 0
.menu.object add separator
.menu.object  add radio -label "Add" -command "object_cb Add" -underline 0
.menu.object add radio -label "Replace" -command "object_cb Replace" -underline 0
#.menu.object  add radio -label "Animate" -command "puts stderr {Animate}" -underline 0
.menu.object  add radio -label "Target" -command "object_cb Target" -underline 0
#.menu.object  add radio -label "Automatic" -command "puts stderr {Automatic}" -underline 0

# Mode Menu: Simple/OP Psurf/OP Psurf-Icurve

menu .menu.mode -takefocus 0
.menu add cascade -label "Mode" -menu .menu.mode -underline 0 
#menubutton .menu.mode -text "Mode"  -menu .menu.mode -underline 0
#menu .menu.mode.m
for {set i 0} {$i < [llength $op_mode_array]} {incr i} {
        .menu.mode add radio -label [lindex $op_mode_array $i] \
         -command op_mode_changed -variable op_mode_index -value $i
        }

# Options Menu: Precision/Colours/Clipping/Course/Fine/Faces/Edges

menu .menu.options -takefocus 0
.menu add cascade -label "Options" -menu .menu.options -underline 1 
#menubutton .menu.options -text "Options" -menu .menu.options -underline 1
#menu .menu.options.m
.menu.options add cascade -label "Precision" \
			-menu .menu.options.precision  -underline 0
.menu.options add cascade -label "Tolerance" \
			-menu .menu.options.tolerance  -underline 0
.menu.options add cascade -label "Itterations" \
	-menu .menu.options.itterations  -underline 0
.menu.options add cascade -label "Colours"  \
	-menu .menu.options.colours  -underline 0

# Precision Sub menu

menu .menu.options.precision
.menu.options.precision add radio -label "0" -variable precision -value 0 -command change_precision
.menu.options.precision add radio -label "1" -variable precision -value 1 -command change_precision
.menu.options.precision add radio -label "2" -variable precision -value 2 -command change_precision
.menu.options.precision add radio -label "3" -variable precision -value 3 -command change_precision
.menu.options.precision add radio -label "4" -variable precision -value 4 -command change_precision
.menu.options.precision add radio -label "5" -variable precision -value 5 -command change_precision
.menu.options.precision add radio -label "6" -variable precision -value 6 -command change_precision
.menu.options.precision add radio -label "7" -variable precision -value 7 -command change_precision
.menu.options.precision add radio -label "8" -variable precision -value 8 -command change_precision
.menu.options.precision add radio -label "9" -variable precision -value 9 -command change_precision

menu .menu.options.tolerance
.menu.options.tolerance add radio -label "0.1"  -value "1.0e-01" \
	  -variable tolerance
.menu.options.tolerance add radio -label "0.01"  -value "1.0e-02" \
	  -variable tolerance
.menu.options.tolerance add radio -label "0.001"  -value "1.0e-03" \
	  -variable tolerance
.menu.options.tolerance add radio -label "0.0001"  -value "1.0e-04" \
	  -variable tolerance
.menu.options.tolerance add radio -label "0.00001"  -value "1.0e-05" \
	  -variable tolerance
.menu.options.tolerance add radio -label "0.000001"  -value "1.0e-06" \
	  -variable tolerance
.menu.options.tolerance add radio -label "0.0000001"  -value "1.0e-07" \
	  -variable tolerance
.menu.options.tolerance add radio -label "0.00000001"  -value "1.0e-08" \
	  -variable tolerance
.menu.options.tolerance add radio -label "0.000000001"  -value "1.0e-09" \
	  -variable tolerance
.menu.options.tolerance add radio -label "0.0000000001"  -value "1.0e-10" \
	  -variable tolerance

menu .menu.options.itterations
.menu.options.itterations add radio -label "1" -variable itterations -value 1 
.menu.options.itterations add radio -label "2" -variable itterations -value 2
.menu.options.itterations add radio -label "5" -variable itterations -value 5
.menu.options.itterations add radio -label "10" -variable itterations \
		 -value 10 
.menu.options.itterations add radio -label "15" -variable itterations \
		 -value 15
.menu.options.itterations add radio -label "20" -variable itterations \
		 -value 20
.menu.options.itterations add radio -label "50" -variable itterations \
		 -value 50 
.menu.options.itterations add radio -label "100" -variable itterations \
		 -value 100

# Colours Sub Menu

menu .menu.options.colours
.menu.options.colours add radio -label "None" -variable colour -value -1 -underline 0
.menu.options.colours add radio -label "Black" -variable colour -value 0 -underline 4
.menu.options.colours add radio -label "Red" -variable colour -value 1 -underline 0
.menu.options.colours add radio -label "Green" -variable colour -value 2 -underline 0
.menu.options.colours add radio -label "Yellow" -variable colour -value 3 -underline 0
.menu.options.colours add radio -label "Blue" -variable colour -value 4 -underline 0
.menu.options.colours add radio -label "Magenta" -variable colour -value 5 -underline 0
.menu.options.colours add radio -label "Cyan" -variable colour -value 6 -underline 0
.menu.options.colours add radio -label "White" -variable colour -value 7 -underline 0


# Equation Button

menu .menu.equation -takefocus 0
.menu add cascade -label "Equation" -menu .menu.equation -underline 0 
#menubutton .menu.equation -text "Equation" -menu .menu.equation -underline 0
#menu .menu.equation 
.menu.equation add command -label "Edit" -command "showEditor" -underline 0
.menu.equation add separator
.menu.equation add command -label "Plane" -command "load_plane" -underline 0
.menu.equation add command -label "Sphere" -command "load_sphere" -underline 0
.menu.equation add command -label "Box" -command "load_box" -underline 0

# Help Button

menu .menu.help -takefocus 0
.menu add cascade -label "Help" -menu .menu.help -underline 0 
#menubutton .menu.help -text "Help" -menu .menu.help -underline 0
#menu .menu.help.m
.menu.help add command -label "General Help" -underline 0 -command mkHelp
.menu.help add command -label "Help on Syntax" -command mkSHelp -underline 8
.menu.help add command -label "Author" -underline 0 -command mkAuth

#pack .menu.file .menu.object .menu.mode .menu.options  .menu.equation -side left
#pack .menu.help -side right
#
## Set up for keyboard-based menu traversal
#
#bind . <Any-FocusIn> {
#    if {("%d" == "NotifyVirtual") && ("%m" == "NotifyNormal")} {
#        focus .menu
#    }
#}
#tk_menuBar .menu .menu.file .menu.object .menu.options .menu.equation .menu.help

. configure -menu .menu
bind . <Shift-Key-Tab> {focus [tk_focusPrev %W]; puts stderr [concat win %W prev [tk_focusPrev %W]]}
bind Menu <Tab> {menuTab %W}

bind . <Tab> {normalTab %W}

proc normalTab w {
        set next [tk_focusNext $w]
        if {[winfo class $next] == "Menu" } {
        #focus $next
                tkFirstMenu $w
        } else {
        focus $next
        }
#       puts stderr [concat win $w next $next]
}

proc menuTab {w} {
#        puts stderr [concat win $w]
	if { $w == ".#menu" } {
		 set w .#menu.#menu#file 
#        	puts stderr [concat win2 $w]
	}
        set l [split $w .]
        #puts stderr [concat l $l]
        set e1 [lindex $l 1]
        #puts stderr [concat top .$e1 ]
        #puts [concat focus [tk_focusNext .$e1]]
        tkMenuUnpost $w
        focus [tk_focusNext .$e1]
}
# Now the main window

frame .r1
frame .r2
pack .r1 .r2  -side top -fill x 
button .r1.run -text "= 0" -command run_tcl -takefocus 1 -highlightthickness 1
button .r1.runp -text ">= 0" -command run_plus_tcl -takefocus 1 -highlightthickness 1
button .r1.runm -text "<= 0" -command run_minus_tcl -takefocus 1 -highlightthickness 1
label  .r1.l1 -text "Target:"
menubutton .r1.mb -textvariable targetname -menu .r1.mb.m -relief sunken
menu .r1.mb.m -postcommand post_targets
#label  .r1.label -textvariable current_file
pack .r1.run .r1.runp .r1.runm .r1.l1 .r1.mb -side left -padx 1m -pady 2m

#label .r2.l1 -text "Mode:"
#menubutton .r2.mb -textvariable op_mode_name -menu .r2.mb.m -relief sunken
#menu .r2.mb.m 
#for {set i 0} {$i < [llength $op_mode_array]} {incr i} {
#        .r2.mb.m add command -label [lindex $op_mode_array $i] \
#         -command [concat "op_mode_changed" $i]
#        }

label  .r2.l2 -text "Psurf:"
menubutton .r2.mb2 -textvariable psurftargetname -menu .r2.mb2.m -relief sunken
menu .r2.mb2.m -postcommand post_psurftargets

label  .r2.l3 -text "Icurve:"
menubutton .r2.mb3 -textvariable icurvetargetname -menu .r2.mb3.m -relief sunken
menu .r2.mb3.m -postcommand post_icurvetargets

label  .r2.l4 -text "Impsurf:"
menubutton .r2.mb4 -textvariable impsurftargetname -menu .r2.mb4.m -relief sunken
menu .r2.mb4.m -postcommand post_impsurftargets

label  .r2.l5 -text "Pcurve:"
menubutton .r2.mb5 -textvariable pcurvetargetname -menu .r2.mb5.m -relief sunken
menu .r2.mb5.m -postcommand post_pcurvetargets


pack .r1.run .r1.runp .r1.runm .r1.l1 .r1.mb -side left -padx 1m -pady 2m
#pack .r2.l1 .r2.mb  
#pack .r2.l2 .r2.mb2 -side left -padx 1m -pady 2m
#pack .r2.l3 .r2.mb3 -side left -padx 1m -pady 2m

set targetname targetgeom
set psurftargetname targetgeom
set impsurftargetname targetgeom
set icurvetargetname targetgeom
set pcurvetargetname targetgeom


mkEditor
#update_parameters
#update_variables
set temp_edit_file /usr/tmp/[pid].aaa
set current_file intersect
#set prog_name [get_prog_name_cb]
set prog_name "intersect"

toplevel .shrink
wm title .shrink "intersect"
wm withdraw .shrink
button .shrink.run -text "= 0" -command run_tcl
button .shrink.runp -text ">= 0" -command run_plus_tcl
button .shrink.runm -text "<= 0" -command run_minus_tcl
button .shrink.grow -text "grow" -command { 
set shrinkgeom [wm geometry .shrink]
wm geometry . [join [list + [lindex $dotgeom 2] + [lindex $dotgeom 3] ] {} ]
wm deiconify . 
wm withdraw .shrink }
pack .shrink.run .shrink.runp .shrink.runm .shrink.grow -side left -padx 1m -pady 2m
set_titles
