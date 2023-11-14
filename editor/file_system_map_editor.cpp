/**************************************************************************/
/*  file_system_map_editor.cpp                                            */
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

#include "file_system_map_editor.h"

#include "editor/editor_scale.h"
#include "editor/event_listener_line_edit.h"
#include "editor/input_event_configuration_dialog.h"
#include "scene/gui/check_button.h"
#include "scene/gui/tree.h"
#include "scene/scene_string_names.h"

static bool _is_file_system_name_valid(const String &p_name) {
	const char32_t *cstr = p_name.get_data();
	for (int i = 0; cstr[i]; i++) {
		if (cstr[i] == '/' || cstr[i] == ':' || cstr[i] == '"' ||
				cstr[i] == '=' || cstr[i] == '\\' || cstr[i] < 32) {
			return false;
		}
	}
	return true;
}

void FileSystemMapEditor::_add_file_system_pressed() {
	_add_file_system(add_edit->get_text());
}

String FileSystemMapEditor::_check_new_file_system_name(const String &p_name) {
	if (p_name.is_empty() || !_is_file_system_name_valid(p_name)) {
		return TTR("Invalid file_system name. It cannot be empty nor contain '/', ':', '=', '\\' or '\"'");
	}

	if (_has_file_system(p_name)) {
		return vformat(TTR("An file_system with the name '%s' already exists."), p_name);
	}

	return "";
}

void FileSystemMapEditor::_add_edit_text_changed(const String &p_name) {
	String error = _check_new_file_system_name(p_name);
	add_button->set_tooltip_text(error);
	add_button->set_disabled(!error.is_empty());
}

bool FileSystemMapEditor::_has_file_system(const String &p_name) const {
	for (const FileSystemInfo &file_system_info : file_systems_cache) {
		if (p_name == file_system_info.name) {
			return true;
		}
	}
	return false;
}

void FileSystemMapEditor::_add_file_system(const String &p_name) {
	String error = _check_new_file_system_name(p_name);
	if (!error.is_empty()) {
		show_message(error);
		return;
	}

	add_edit->clear();
	emit_signal(SNAME("file_system_added"), p_name);
}

void FileSystemMapEditor::_file_system_edited() {
	TreeItem *ti = file_system_tree->get_edited();
	if (!ti) {
		return;
	}

	if (file_system_tree->get_selected_column() == 0) {
		// Name Edited
		String new_name = ti->get_text(0);
		String old_name = ti->get_meta("__name");

		if (new_name == old_name) {
			return;
		}

		if (new_name.is_empty() || !_is_file_system_name_valid(new_name)) {
			ti->set_text(0, old_name);
			show_message(TTR("Invalid file_system name. It cannot be empty nor contain '/', ':', '=', '\\' or '\"'"));
			return;
		}

		if (_has_file_system(new_name)) {
			ti->set_text(0, old_name);
			show_message(vformat(TTR("An file_system with the name '%s' already exists."), new_name));
			return;
		}

		emit_signal(SNAME("file_system_renamed"), old_name, new_name);
	} else if (file_system_tree->get_selected_column() == 1) {
		// path Edited
		String name = ti->get_meta("__name");
		Dictionary old_file_system = ti->get_meta("__file_system");
		Dictionary new_file_system = old_file_system.duplicate();
		new_file_system["path"] = ti->get_range(1);

		// Call deferred so that input can finish propagating through tree, allowing re-making of tree to occur.
		call_deferred(SNAME("emit_signal"), "file_system_edited", name, new_file_system);
	}
}

void FileSystemMapEditor::_tree_button_pressed(Object *p_item, int p_column, int p_id, MouseButton p_button) {
	if (p_button != MouseButton::LEFT) {
		return;
	}

	ItemButton option = (ItemButton)p_id;

	TreeItem *item = Object::cast_to<TreeItem>(p_item);
	if (!item) {
		return;
	}

	switch (option) {
		case FileSystemMapEditor::BUTTOM_REMOVE_FILE_SYSTEM: {
			// Send removed file_system name
			String name = item->get_meta("__name");
			emit_signal(SNAME("file_system_removed"), name);
		} break;
		case FileSystemMapEditor::BUTTOM_REVERT_FILE_SYSTEM: {
			ERR_FAIL_COND_MSG(!item->has_meta("__file_system_initial"), "Tree Item for file_system which can be reverted is expected to have meta value with initial value of file_system.");

			Dictionary file_system = item->get_meta("__file_system_initial").duplicate();
			String file_system_name = item->get_meta("__name");

			emit_signal(SNAME("file_system_edited"), file_system_name, file_system);
		} break;
		default:
			break;
	}
}

// void FileSystemMapEditor::_tree_item_activated() {
// 	TreeItem *item = file_system_tree->get_selected();

// 	if (!item || !item->has_meta("__event")) {
// 		return;
// 	}

// 	_tree_button_pressed(item, 2, BUTTON_EDIT_EVENT, MouseButton::LEFT);
// }

void FileSystemMapEditor::set_show_builtin_file_systems(bool p_show) {
	show_builtin_file_systems = p_show;
	show_builtin_file_systems_checkbutton->set_pressed(p_show);

	// Prevent unnecessary updates of file_system list when cache is empty.
	if (!file_systems_cache.is_empty()) {
		update_file_system_list();
	}
}

void FileSystemMapEditor::_search_term_updated(const String &) {
	update_file_system_list();
}

Variant FileSystemMapEditor::get_drag_data_fw(const Point2 &p_point, Control *p_from) {
	TreeItem *selected = file_system_tree->get_selected();
	if (!selected) {
		return Variant();
	}

	String name = selected->get_text(0);
	Label *label = memnew(Label(name));
	label->set_theme_type_variation("HeaderSmall");
	label->set_modulate(Color(1, 1, 1, 1.0f));
	file_system_tree->set_drag_preview(label);

	Dictionary drag_data;

	if (selected->has_meta("__file_system")) {
		drag_data["input_type"] = "file_system";
	}

	if (selected->has_meta("__event")) {
		drag_data["input_type"] = "event";
	}

	file_system_tree->set_drop_mode_flags(Tree::DROP_MODE_INBETWEEN);

	return drag_data;
}

bool FileSystemMapEditor::can_drop_data_fw(const Point2 &p_point, const Variant &p_data, Control *p_from) const {
	Dictionary d = p_data;
	if (!d.has("input_type")) {
		return false;
	}

	TreeItem *selected = file_system_tree->get_selected();
	TreeItem *item = file_system_tree->get_item_at_position(p_point);
	if (!selected || !item || item == selected) {
		return false;
	}

	// Don't allow moving an file_system in-between events.
	if (d["input_type"] == "file_system" && item->has_meta("__event")) {
		return false;
	}

	// Don't allow moving an event to a different file_system.
	if (d["input_type"] == "event" && item->get_parent() != selected->get_parent()) {
		return false;
	}

	return true;
}

void FileSystemMapEditor::drop_data_fw(const Point2 &p_point, const Variant &p_data, Control *p_from) {
	if (!can_drop_data_fw(p_point, p_data, p_from)) {
		return;
	}

	TreeItem *selected = file_system_tree->get_selected();
	TreeItem *target = file_system_tree->get_item_at_position(p_point);
	bool drop_above = file_system_tree->get_drop_section_at_position(p_point) == -1;

	if (!target) {
		return;
	}

	Dictionary d = p_data;
	if (d["input_type"] == "file_system") {
		// Change file_system order.
		String relative_to = target->get_meta("__name");
		String file_system_name = selected->get_meta("__name");
		emit_signal(SNAME("file_system_reordered"), file_system_name, relative_to, drop_above);
	}
}

void FileSystemMapEditor::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE:
		case NOTIFICATION_THEME_CHANGED: {
			file_system_list_search->set_right_icon(get_theme_icon(SNAME("Search"), SNAME("EditorIcons")));
			if (!file_systems_cache.is_empty()) {
				update_file_system_list();
			}
		} break;
	}
}

void FileSystemMapEditor::_bind_methods() {
	ADD_SIGNAL(MethodInfo("file_system_added", PropertyInfo(Variant::STRING, "name")));
	ADD_SIGNAL(MethodInfo("file_system_edited", PropertyInfo(Variant::STRING, "name"), PropertyInfo(Variant::DICTIONARY, "new_file_system")));
	ADD_SIGNAL(MethodInfo("file_system_removed", PropertyInfo(Variant::STRING, "name")));
	ADD_SIGNAL(MethodInfo("file_system_renamed", PropertyInfo(Variant::STRING, "old_name"), PropertyInfo(Variant::STRING, "new_name")));
	ADD_SIGNAL(MethodInfo("file_system_reordered", PropertyInfo(Variant::STRING, "file_system_name"), PropertyInfo(Variant::STRING, "relative_to"), PropertyInfo(Variant::BOOL, "before")));
	ADD_SIGNAL(MethodInfo(SNAME("filter_focused")));
	ADD_SIGNAL(MethodInfo(SNAME("filter_unfocused")));
}

LineEdit *FileSystemMapEditor::get_search_box() const {
	return file_system_list_search;
}

InputEventConfigurationDialog *FileSystemMapEditor::get_configuration_dialog() {
	return event_config_dialog;
}

void FileSystemMapEditor::update_file_system_list(const Vector<FileSystemInfo> &p_file_system_infos) {
	if (!p_file_system_infos.is_empty()) {
		file_systems_cache = p_file_system_infos;
	}

	file_system_tree->clear();
	TreeItem *root = file_system_tree->create_item();

	for (int i = 0; i < file_systems_cache.size(); i++) {
		FileSystemInfo file_system_info = file_systems_cache[i];

		if (!file_system_info.editable && !show_builtin_file_systems) {
			continue;
		}

		const Variant path = file_system_info.file_system["path"];

		// Update Tree...

		TreeItem *file_system_item = file_system_tree->create_item(root);
		file_system_item->set_meta("__file_system", file_system_info.file_system);
		file_system_item->set_meta("__name", file_system_info.name);

		// First Column - FileSystem Name
		file_system_item->set_text(0, file_system_info.name);
		file_system_item->set_editable(0, file_system_info.editable);
		// file_system_item->set_icon(0, file_system_info.icon);

		// Second Column - path
		file_system_item->set_editable(1, true);
		file_system_item->set_cell_mode(1, TreeItem::CELL_MODE_STRING);
		file_system_item->set_range_config(1, 0.0, 1.0, 0.01);
		file_system_item->set_range(1, path);

		// Third column - buttons
		// if (file_system_info.has_initial) {
		// 	bool path_eq = file_system_info.file_system_initial["path"] == file_system_info.file_system["path"];
		// 	// bool events_eq = Shortcut::is_event_array_equal(file_system_info.file_system_initial["events"], file_system_info.file_system["events"]);
		// 	bool file_system_eq = path_eq; // && events_eq;
		// 	file_system_item->set_meta("__file_system_initial", file_system_info.file_system_initial);
		// 	file_system_item->add_button(2, file_system_tree->get_theme_icon(SNAME("ReloadSmall"), SNAME("EditorIcons")), BUTTOM_REVERT_FILE_SYSTEM, file_system_eq, file_system_eq ? TTR("Cannot Revert - FileSystem is same as initial") : TTR("Revert FileSystem"));
		// }
		file_system_item->add_button(2, file_system_tree->get_theme_icon(SNAME("Remove"), SNAME("EditorIcons")), BUTTOM_REMOVE_FILE_SYSTEM, !file_system_info.editable, file_system_info.editable ? TTR("Remove FileSystem") : TTR("Cannot Remove FileSystem"));

		file_system_item->set_custom_bg_color(0, file_system_tree->get_theme_color(SNAME("prop_subsection"), SNAME("Editor")));
		file_system_item->set_custom_bg_color(1, file_system_tree->get_theme_color(SNAME("prop_subsection"), SNAME("Editor")));
	}
}

void FileSystemMapEditor::show_message(const String &p_message) {
	message->set_text(p_message);
	message->popup_centered();
}

void FileSystemMapEditor::use_external_search_box(LineEdit *p_searchbox) {
	memdelete(file_system_list_search);
	file_system_list_search = p_searchbox;
	file_system_list_search->connect("text_changed", callable_mp(this, &FileSystemMapEditor::_search_term_updated));
}

void FileSystemMapEditor::_on_filter_focused() {
	emit_signal(SNAME("filter_focused"));
}

void FileSystemMapEditor::_on_filter_unfocused() {
	emit_signal(SNAME("filter_unfocused"));
}

FileSystemMapEditor::FileSystemMapEditor() {
	// Main Vbox Container
	VBoxContainer *main_vbox = memnew(VBoxContainer);
	main_vbox->set_anchors_and_offsets_preset(PRESET_FULL_RECT);
	add_child(main_vbox);

	HBoxContainer *top_hbox = memnew(HBoxContainer);
	main_vbox->add_child(top_hbox);

	file_system_list_search = memnew(LineEdit);
	file_system_list_search->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	file_system_list_search->set_placeholder(TTR("Filter by name..."));
	file_system_list_search->set_clear_button_enabled(true);
	file_system_list_search->connect("text_changed", callable_mp(this, &FileSystemMapEditor::_search_term_updated));
	top_hbox->add_child(file_system_list_search);


	Button *clear_all_search = memnew(Button);
	clear_all_search->set_text(TTR("Clear All"));
	clear_all_search->connect("pressed", callable_mp(file_system_list_search, &LineEdit::clear));
	top_hbox->add_child(clear_all_search);

	// Adding FileSystem line edit + button
	add_hbox = memnew(HBoxContainer);
	add_hbox->set_h_size_flags(Control::SIZE_EXPAND_FILL);

	add_edit = memnew(LineEdit);
	add_edit->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	add_edit->set_placeholder(TTR("Add New FileSystem"));
	add_edit->set_clear_button_enabled(true);
	add_edit->connect("text_changed", callable_mp(this, &FileSystemMapEditor::_add_edit_text_changed));
	add_edit->connect("text_submitted", callable_mp(this, &FileSystemMapEditor::_add_file_system));
	add_hbox->add_child(add_edit);

	add_button = memnew(Button);
	add_button->set_text(TTR("Add"));
	add_button->connect("pressed", callable_mp(this, &FileSystemMapEditor::_add_file_system_pressed));
	add_hbox->add_child(add_button);
	// Disable the button and set its tooltip.
	_add_edit_text_changed(add_edit->get_text());

	show_builtin_file_systems_checkbutton = memnew(CheckButton);
	show_builtin_file_systems_checkbutton->set_pressed(false);
	show_builtin_file_systems_checkbutton->set_text(TTR("Show Built-in FileSystems"));
	show_builtin_file_systems_checkbutton->connect("toggled", callable_mp(this, &FileSystemMapEditor::set_show_builtin_file_systems));
	add_hbox->add_child(show_builtin_file_systems_checkbutton);

	main_vbox->add_child(add_hbox);

	// FileSystem Editor Tree
	file_system_tree = memnew(Tree);
	file_system_tree->set_v_size_flags(Control::SIZE_EXPAND_FILL);
	file_system_tree->set_columns(3);
	file_system_tree->set_hide_root(true);
	file_system_tree->set_column_titles_visible(true);
	file_system_tree->set_column_title(0, TTR("FileSystem"));
	file_system_tree->set_column_clip_content(0, true);
	file_system_tree->set_column_title(1, TTR("path"));
	file_system_tree->set_column_expand(1, false);
	file_system_tree->set_column_custom_minimum_width(1, 80 * EDSCALE);
	file_system_tree->set_column_expand(2, false);
	file_system_tree->set_column_custom_minimum_width(2, 50 * EDSCALE);
	file_system_tree->connect("item_edited", callable_mp(this, &FileSystemMapEditor::_file_system_edited));
	file_system_tree->connect("item_activated", callable_mp(this, &FileSystemMapEditor::_tree_item_activated));
	file_system_tree->connect("button_clicked", callable_mp(this, &FileSystemMapEditor::_tree_button_pressed));
	main_vbox->add_child(file_system_tree);

	SET_DRAG_FORWARDING_GCD(file_system_tree, FileSystemMapEditor);

	// Adding event dialog
	event_config_dialog = memnew(InputEventConfigurationDialog);
	event_config_dialog->connect("confirmed", callable_mp(this, &FileSystemMapEditor::_event_config_confirmed));
	add_child(event_config_dialog);

	message = memnew(AcceptDialog);
	add_child(message);
}
