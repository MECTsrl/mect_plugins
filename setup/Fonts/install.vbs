Set objShellApp = CreateObject("Shell.Application")
Set objFSO = CreateObject("Scripting.FileSystemObject")
Const FONTS = &H14&
Set objFolder = objShellApp.Namespace(FONTS)
strNewFontsFolder = WScript.Arguments(0) & "\Fonts"
If objFSO.FolderExists(strNewFontsFolder) = True Then
	For Each objFile In objFSO.GetFolder(strNewFontsFolder).Files
		If Right(LCase(objFile.Name), 4) = ".ttf" Then
			If objFSO.FileExists(objFolder.Self.Path & "\" & objFile.Name) = False Then objFolder.CopyHere objFile.Path
		End If
	Next
Else
	MsgBox "Unable to find " & strWindowsFonts
End If