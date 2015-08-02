<p align='right'> <b>10/10/2009</b>
</p>


---

Like other EDA Toped allows different aspects of user interface to be customised


### 1. Customizing menu and access-keys ###

addmenu function

This function provides possibility to mix build-in and user-defined menus.

Lets consider typical use cases:
```
addmenu("Scripts/myscript", "F2", "myscript();");
```

Create new top menu item “Script” and sub-menu “myscript”. Selecting this item or pressing F2 key will call user-defined tell-function “myscript();”. Note that in the main menu the item “Script” appears before “Help” so we still keep the common viewstyle with right-most “Help” menu item.



It is not necessary to create a tell-function every time when we want to add a new menu item.

```
addmenu("Draw/MyBox", "CTRL-W", "addbox({0,0}, {10,10});");
```

Having a "Draw" menu existing, "MyBox" item will be added, with "control-w" hot-key.



Hot-key specification can be skipped.



### 2. Working with toolbars ###

There are four possible sizes for toolbars: 16x16, 24x24, 32x32, 48x48 pixels. The toolbar size can be changed using menu Settings->Toolbar Size.

All icons must be in png format.



Every toolbar in toped has a name. Currently we have “main” toolbar that contains “new cell”, “open cell”, “save design” items, and “edit” toolbar with “undo”, “add box” etc. Function **toolbaradditem** allows to add new item to an existing toolbar or (in case there is no such name) to a new one.

There are two forms of this function

```
void toolbaradditem(string tbarname, strmap item);

void toolbaradditem(string tbarname, strmap list );
```


Lets consider the first form. Tbarname is the toolbar name described above. Item is a list consisting of two strings. First one is the item name, second one is the corresponding tell-function.

Example:
```
toolbaradditem("main",{"redo","redo();"});
```

Now near the save button a new button will appear. How does it it work? The toolbaritem function looks for icon files in $TPD\_GLOBAL/icons directory. Files with names “redo16x16.png”, “redo24x24.png”, “redo32x32.png”, “redo48x48.png” will be loaded. If there is no file for particular icon size, Toped will use the nearest one.

The second form of the **toolbaradditem** function allows to set several icons at once.
```
toolbaradditem("my",{{"redo","redo();"},
      {“undo”, “undo()”},
      {“zoom_all”, “zoomall();”}});
```

You can delete any toolbar item by providing the toolbar name and the item name
```
toolbardeleteitem(string tbarname, string iconname);
```