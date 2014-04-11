; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CRecordAudioDlg
LastTemplate=generic CWnd
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "RecordAudio.h"

ClassCount=4
Class1=CRecordAudioApp
Class2=CRecordAudioDlg
Class3=CAboutDlg

ResourceCount=3
Resource1=IDD_ABOUTBOX
Resource2=IDR_MAINFRAME
Class4=CAudioCtrl
Resource3=IDD_RECORDAUDIO_DIALOG

[CLS:CRecordAudioApp]
Type=0
HeaderFile=RecordAudio.h
ImplementationFile=RecordAudio.cpp
Filter=N

[CLS:CRecordAudioDlg]
Type=0
HeaderFile=RecordAudioDlg.h
ImplementationFile=RecordAudioDlg.cpp
Filter=D
LastObject=CRecordAudioDlg
BaseClass=CDialog
VirtualFilter=dWC

[CLS:CAboutDlg]
Type=0
HeaderFile=RecordAudioDlg.h
ImplementationFile=RecordAudioDlg.cpp
Filter=D

[DLG:IDD_ABOUTBOX]
Type=1
Class=CAboutDlg
ControlCount=4
Control1=IDC_STATIC,static,1342177283
Control2=IDC_STATIC,static,1342308480
Control3=IDC_STATIC,static,1342308352
Control4=IDOK,button,1342373889

[DLG:IDD_RECORDAUDIO_DIALOG]
Type=1
Class=CRecordAudioDlg
ControlCount=17
Control1=IDOK,button,1342242817
Control2=IDC_STOP,button,1342242816
Control3=IDC_STATIC,static,1342308352
Control4=IDC_CMB_IN_NAMES,combobox,1344339971
Control5=IDC_SLIDER_RECORD,msctls_trackbar32,1342242840
Control6=IDC_SLIDER_PLAY,msctls_trackbar32,1342242840
Control7=IDC_SATC_GRAPH,static,1342177287
Control8=IDC_BUTTON1,button,1342242816
Control9=IDC_BUTTON2,button,1342242816
Control10=IDC_STATIC,static,1342308352
Control11=IDC_CMB_DEVS_NAME,combobox,1344339971
Control12=IDC_STATIC,static,1342308352
Control13=IDC_STATIC,static,1342308352
Control14=IDC_STATIC,button,1342177287
Control15=IDC_STATIC,button,1342177287
Control16=IDC_STATIC,static,1342308352
Control17=IDC_CMB_DEVS_PLAY,combobox,1344339971

[CLS:CAudioCtrl]
Type=0
HeaderFile=AudioCtrl.h
ImplementationFile=AudioCtrl.cpp
BaseClass=CWnd
Filter=W
LastObject=CAudioCtrl
VirtualFilter=WC

