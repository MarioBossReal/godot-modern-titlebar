@tool
extends EditorPlugin

var plugin : ModernTitleBar

func _enter_tree() -> void:
	if (!Engine.is_editor_hint()):
		return
		
	plugin = ModernTitleBar.new();
	get_tree().root.add_child(plugin)


func _exit_tree() -> void:
	if (!Engine.is_editor_hint()):
		return
	
	if (plugin == null):
		return
		
	plugin.queue_free()
