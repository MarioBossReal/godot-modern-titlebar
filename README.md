# Godot Modern Titlebar
A modern titlebar redesign for Godot 4.6 written in C# for Windows 11 and 10

This is only a prototype, and currently only supports Windows 11 and 10.
It is being developed mainly for Windows 11, however it has been tested on Windows 10 and works well except
that it removes the native 1px window border and subwindow titlebars aren't colour matched to the editor's colour.

For this prototype, the main editor window has the native titlebar removed, and the window buttons (minimise, maximise, close)
are recreated accurately using control nodes.

In the future, I want to test out how well Godot reacts to extending the client area into the non-client area.
If it reacts well, then the native titlebar can be used and the window buttons won't have to be recreated, which would be ideal for
preserving more native system functions and would pave the way for allowing non-client drawing in the editor for other operating systems.

<img width="1971" height="1192" alt="image" src="https://github.com/user-attachments/assets/9e527c2a-c090-4c97-ba2b-3bf75459f070" />

# Before
<img width="1376" height="147" alt="image" src="https://github.com/user-attachments/assets/6cae066e-f39b-4b92-8fd0-fcda532c00eb" />

# After
<img width="1390" height="105" alt="image" src="https://github.com/user-attachments/assets/1adce6be-ee2f-47bf-ba3f-498853f2177b" />

# MenuBar
<img width="406" height="615" alt="image" src="https://github.com/user-attachments/assets/1363329c-dea5-4a63-a10a-900f555999b1" /><img width="406" height="615" alt="image" src="https://github.com/user-attachments/assets/f56f1e3a-3323-48e3-99a9-df3636378633" />


