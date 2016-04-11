# protect-process
Protect process fsfilter driver. Windows x64 


Makes use a fsfilter driver to apply irp on your process and deny access.


#example


```

if (wcsstr(nameInfo->Name.Buffer, L"your_file.exe") != NULL)
{
	Data->IoStatus.Status = STATUS_NO_SUCH_FILE;
	Data->IoStatus.Information = 0;
	return FLT_PREOP_COMPLETE;
}

```

# contact
0xdr@protonmail.ch
> Dr. R

