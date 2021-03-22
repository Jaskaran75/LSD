#*************************************************************
#
#	LSD 8.0 - December 2020
#	written by Marco Valente, Universita' dell'Aquila
#	and by Marcelo Pereira, University of Campinas
#
#	Copyright Marco Valente and Marcelo Pereira
#	LSD is distributed under the GNU General Public License
#
#	See Readme.txt for copyright information of
#	third parties' code used in LSD
#	
#*************************************************************

#*************************************************************
# WGTCLONE.TCL
# Collection of procedures to clone, move, copy, serialize
# Tk widgets.
#
# Based on code by Cjolly 
# https://stackoverflow.com/questions/6285648
# Based on code Maurice Bredelet
# https://wiki.tcl-lang.org/page/Serializing+a+canvas+widget
#*************************************************************

#************************************************
# CLONE_WIDGET/MOVE_WIDGET
# Clone or move an entire widget to another
# toplevel parent
#************************************************
proc getWidgetType { w } {
	set class [ winfo class $w ]
	
	if { [ string index $class 0 ] eq "T" &&
		 [ string match "\[A-Z\]" [ string index $class 1 ] ] } {
			set class [ string range [ string tolower $class ] 1 end ]
			set class "ttk::$class"
	} else {
		set class [ string tolower $class ]
	}
	
	return $class
}

proc getConfigOptions { w } {
	set configure [ $w configure ]
	set options { }
	
	foreach f $configure {
		if { [ llength $f ] < 3 } { 
			continue 
		} 
		
		set name [ lindex $f 0 ]
		set default [ lindex $f end-1 ]
		set value [ lindex $f end ]
		
		if { $default ne $value } {
			lappend options $name $value 
		}
	}
	
	return $options
}

proc clone_widget { w  newparent { level 0 } } {
	set type [ getWidgetType $w ]
	set name [ string trimright $newparent.[ lindex [ split $w "." ] end ] "." ]  

	if { $type eq "canvas" } {
		set retval [ clone_canvas $w $name ]
	} else {
		set retval [ $type $name {*}[ getConfigOptions $w ] ]
	}
	
	foreach b [ bind $w ] {
		puts "bind $retval $b [ subst { [ bind $w $b ] } ] " 
		catch { 
			bind $retval $b [ subst { [ bind $w $b ] } ]
		} 
	} 
	
	if { $level > 0 } {
		if { [ catch { pack info $w } err ] == 0 } {
			array set temp [ pack info $w ]
			array unset temp -in
			catch { 
				pack $name {*}[ array get temp ] 
			} 
		} elseif { [ catch { grid info $w } err ] == 0 } {
			array set temp [ grid info $w ]
			array unset temp -in
			catch { 
				grid $name {*}[ array get temp ] 
			} 
		}
	}
	
	incr level 
	
	if { [ pack slaves $w ] ne "" } { 
		foreach f [ pack slaves $w ] {
			clone_widget $f $name $level
		}
	} else {
		foreach f [ winfo children $w ] {
			clone_widget $f $name $level
		}
	}
	
	return $retval
}

proc move_widget { w newparent } {
	set retval [ clone_widget $w $newparent ]
	update idletasks
	destroy $w
	return $retval
}


#************************************************
# CLONE_CANVAS/MOVE_CANVAS
# Clone a canvas widget
#************************************************
proc clone_canvas { canvas clone } { 
	set retval [ restore_canvas $clone [ save_canvas $canvas ] ]
	return $retval
}

proc move_canvas { canvas clone } { 
	set retval [ clone_canvas $canvas $clone ]
	update idletasks
	destroy $canvas
	return $retval
}


#************************************************
# SAVE_CANVAS
# Serialize a canvas widget
#************************************************
proc options { options } {
	set res { }
	
	foreach option $options {
		set key [ lindex $option 0 ]
		set value [ lindex $option 4 ]
		
		if { [ llength $option ] == 5 } {
			lappend res [ list $key $value ]
		}
  }
  
  return $res
}

proc save_canvas { w } {
	
	lappend save $w; # canvas name
	lappend save [ options [ $w configure ] ]; # canvas option
	lappend save [ $w focus ]; # canvas focus
	
	# canvas items
	foreach id [ $w find all ] {
		set item { }
		set binds { }
		set specifics { }
		set type [ $w type $id ]
		set tags [ $w gettags $id ]
		
		lappend item [ list $type $id ]; # type & id
		lappend item [ $w coords $id ]; # coords
		lappend item $tags; # tags
		
		
		# id binds
		set events [ $w bind $id ]
		foreach event $events { 
			lappend binds [ list $id $event [ $w bind $id $event ] ] 
		}
		
		# tags binds
		foreach tag $tags { 
			set events [ $w bind $tag ]
			foreach event $events { 
				lappend binds [ list $tag $event [ $w bind $tag $event ] ]
			}
		}
		
		lappend item $binds; # binds
		lappend item [ options [ $w itemconfigure $id ] ]; # options
		
		switch $type {
			arc -
			bitmap -
			line -
			oval -
			polygon -
			rectangle { }
			
			image {
				set iname [ $w itemcget $id -image ]
				lappend specifics $iname; # image name
				lappend specifics [ image type $iname ]; # image type
				lappend specifics [ options [ $iname configure ] ]; # image options
			}
			
			text {
				foreach index { insert sel.first sel.last } {
					catch { 
						lappend specifics [ $w index $id $index ]; # text indexes
					}
				}
			}
			
			window {
				
				set wname [ $w itemcget $id -window ]
				lappend specifics $wname; # window name
				lappend specifics [ string tolower [ winfo class $wname ] ]; # window type
				lappend specifics [ options [ $wname configure ] ]; # window options
			}
		}
		
		lappend item $specifics; # type specifics
		
		lappend save $item; # item in serialized canvas
	}

	return $save
}


#************************************************
# RESTORE_CANVAS
# Restore a serialized canvas widget
#************************************************
proc restore_canvas { w save } {
	
	eval canvas $w [ join [ lindex $save 1 ] ]; # create canvas with options
	
	# restore items
	foreach item [ lrange $save 3 end ] {
		foreach { typeid coords tags binds options specifics } $item {
			
			set type [ lindex $typeid 0 ]; # get type
			
			# create bitmap or window
			switch $type {
				image {
					foreach { iname itype ioptions } $specifics {
						break
					}
					
					if { ! [ image inuse $iname] } { 
						eval image create $itype $iname [ join $ioptions ]
					}
				}
				
				window {
					foreach { wname wtype woptions } $specifics {
						break
					}
					
					if { ! [ winfo exists $wname ] } { 
						eval $wtype $wname [ join $woptions ]
					}
					
					raise $wname
				}
			}
			
			# create item
			set id [ eval $w create $type $coords -tags "{$tags}" [ join $options ] ]
			
			foreach bind $binds { 
				foreach { id event script } $bind { 
					catch {
						$w bind $id $event $script; # item bindings
					}
				}
			}
			
			# item specifics
			if { $specifics != "" } {
				switch $type {
					text {
						foreach { insert sel.first sel.last } $specifics {
							break
						}
						
						$w icursor $id $insert 
						
						if { ${sel.first} != "" } {
							$w select from $id ${sel.first}
							$w select to $id ${sel.last}
						}
					}
				}
			}
		}
	}
	
	# restore focused item
	set focus [ lindex $save 2 ]
	if { $focus != "" } {
		$w focus [ lindex $save 2 ]
		focus $w
	}

	return $w
}


#************************************************
# DUMP_CANVAS
# Dump a canvas widget (for saving)
#************************************************
proc dump_canvas { w } {
	set w [ save_canvas $w ]
	
	lappend res [ lindex $w 0 ]; # canvas name
	
	foreach option [ lindex $w 1 ] { 
		lappend res [ join $option \t ]; # canvas options
	}
	
	lappend res [ join [ lindex $w 2 ] \t ]; # focused item
	
	# items
	foreach item [ lrange $w 3 end ] {
		foreach { type coords tags binds options specifics } $item {
			lappend res [ join $type \t ]; # item type
			lappend res \tcoords\t$coords; # item coords
			lappend res \ttags\t$tags; # item tags
			
			lappend res \tbinds; # item bindings
			foreach bind $binds { 
				lappend res \t\t$bind
			}
			
			
			lappend res \toptions; # item options
			foreach option $options {
				set key [ lindex $option 0 ]
				set value [ lindex $option 1 ]
				lappend res \t\t$key\t$value
			}
			
			# item specifics
			if { $specifics != "" } {
				lappend res \tspecifics
				foreach specific $specifics { 
					if { [ llength $specific ] == 1 }  { 
						lappend res \t\t$specific
					} else { 
						foreach token $specific { 
							lappend res \t\t$token 
						}
					}
				}
			}
		}
	}

	return [ join $res \n ]
}