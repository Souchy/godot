/**************************************************************************/
/*  file_system_map_editor.h                                              */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#ifndef FILE_SYSTEM_MAP_EDITOR_H
#define FILE_SYSTEM_MAP_EDITOR_H

#include "scene/gui/control.h"

class Button;
class HBoxContainer;
class EventListenerLineEdit;
class LineEdit;
class CheckButton;
class AcceptDialog;
class InputEventConfigurationDialog;
class Tree;

class FileSystemMapEditor : public Control {
	GDCLASS(FileSystemMapEditor, Control);

public:
	struct FileSystemInfo {
		String name;
		// String path;
		Dictionary file_system;

		bool editable = true;
		// TODO import settings for each file system
	};

private:
	enum ItemButton {
		BUTTOM_REMOVE_FILE_SYSTEM,
		BUTTOM_REVERT_FILE_SYSTEM,
	};

	Vector<FileSystemInfo> file_systems_cache;
	Tree *file_system_tree = nullptr;

	// Storing which file_system/event is currently being edited in the InputEventConfigurationDialog.

	Dictionary current_file_system;
	String current_file_system_name;
	int current_file_system_event_index = -1;

	// Popups

	InputEventConfigurationDialog *event_config_dialog = nullptr;
	AcceptDialog *message = nullptr;

	// Filtering and Adding file_systems

	bool show_builtin_file_systems = false;
	CheckButton *show_builtin_file_systems_checkbutton = nullptr;
	LineEdit *file_system_list_search = nullptr;

	HBoxContainer *add_hbox = nullptr;
	LineEdit *add_edit = nullptr;
	Button *add_button = nullptr;

	void _event_config_confirmed();

	void _add_file_system_pressed();
	void _add_edit_text_changed(const String &p_name);
	String _check_new_file_system_name(const String &p_name);
	bool _has_file_system(const String &p_name) const;
	void _add_file_system(const String &p_name);
	void _file_system_edited();

	void _tree_button_pressed(Object *p_item, int p_column, int p_id, MouseButton p_button);
	void _tree_item_activated();
	void _search_term_updated(const String &p_search_term);
	bool _should_display_file_system(const String &p_name, const Array &p_events) const;

	Variant get_drag_data_fw(const Point2 &p_point, Control *p_from);
	bool can_drop_data_fw(const Point2 &p_point, const Variant &p_data, Control *p_from) const;
	void drop_data_fw(const Point2 &p_point, const Variant &p_data, Control *p_from);

	void _on_filter_focused();
	void _on_filter_unfocused();

protected:
	void _notification(int p_what);
	static void _bind_methods();

public:
	LineEdit *get_search_box() const;
	InputEventConfigurationDialog *get_configuration_dialog();

	// Dictionary represents a FileSystem with "events" (Array) and "deadzone" (float) items. Pass with no param to update list from cached file_system map.
	void update_file_system_list(const Vector<FileSystemInfo> &p_file_system_infos = Vector<FileSystemInfo>());
	void show_message(const String &p_message);

	void set_show_builtin_file_systems(bool p_show);

	void use_external_search_box(LineEdit *p_searchbox);

	FileSystemMapEditor();
};

#endif // FILE_SYSTEM_MAP_EDITOR_H
