object Options: TOptions
  Left = 0
  Top = 0
  BorderStyle = bsToolWindow
  Caption = '  Boxon Options Setup'
  ClientHeight = 666
  ClientWidth = 298
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  Font.Quality = fqClearType
  OldCreateOrder = False
  Position = poScreenCenter
  PixelsPerInch = 96
  TextHeight = 13
  object Label3: TLabel
    Left = 0
    Top = 0
    Width = 298
    Height = 25
    Align = alTop
    Alignment = taCenter
    Caption = 'Music folder'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -21
    Font.Name = 'Tahoma'
    Font.Style = []
    Font.Quality = fqClearType
    ParentFont = False
    ExplicitWidth = 113
  end
  object LbMusicPath: TLabel
    Left = 0
    Top = 25
    Width = 298
    Height = 13
    Align = alTop
    Alignment = taCenter
    Caption = 'LbMusicPath'
    ExplicitWidth = 59
  end
  object Label5: TLabel
    Left = -2
    Top = 265
    Width = 300
    Height = 25
    Align = alCustom
    Alignment = taCenter
    AutoSize = False
    Caption = 'Playlist'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -21
    Font.Name = 'Tahoma'
    Font.Style = []
    Font.Quality = fqClearType
    ParentFont = False
  end
  object LbRandomTrack: TLabel
    Left = 49
    Top = 615
    Width = 199
    Height = 19
    Caption = '5 Random Tracks preloaded'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -16
    Font.Name = 'Tahoma'
    Font.Style = []
    Font.Quality = fqClearType
    ParentFont = False
  end
  object Label2: TLabel
    Left = 6
    Top = 651
    Width = 284
    Height = 13
    Alignment = taCenter
    AutoSize = False
    Caption = #169' B.Tran 2020 - bernard_tran@hotmail.com'
  end
  object LabelProgressValid: TLabel
    Left = 8
    Top = 141
    Width = 254
    Height = 13
    Caption = 'Cliquer sur Verify Valid Audio Files pour lancer le scan'
  end
  object GroupBox1: TGroupBox
    Left = 6
    Top = 399
    Width = 284
    Height = 153
    Caption = ' Fonts '
    TabOrder = 0
    object LbMinSize: TLabel
      Left = 32
      Top = 47
      Width = 62
      Height = 13
      Caption = 'Minimum Size'
    end
    object LbMaxSize: TLabel
      Left = 160
      Top = 47
      Width = 66
      Height = 13
      Caption = 'Maximum Size'
    end
    object RbFixed: TRadioButton
      Left = 16
      Top = 24
      Width = 113
      Height = 17
      Caption = 'Fixed'
      TabOrder = 0
      OnClick = RbFixedClick
    end
    object RbResize: TRadioButton
      Left = 144
      Top = 24
      Width = 113
      Height = 17
      Caption = ' Proportional'
      Checked = True
      TabOrder = 1
      TabStop = True
      OnClick = RbResizeClick
    end
    object ScrollBarMin: TScrollBar
      Left = 16
      Top = 64
      Width = 121
      Height = 17
      PageSize = 0
      TabOrder = 2
    end
    object ScrollBarMax: TScrollBar
      Left = 143
      Top = 64
      Width = 121
      Height = 17
      PageSize = 0
      TabOrder = 3
    end
    object PanelFontMin: TPanel
      Left = 16
      Top = 88
      Width = 121
      Height = 57
      BevelOuter = bvLowered
      Caption = 'Abc'
      TabOrder = 4
    end
    object PanelFontMax: TPanel
      Left = 143
      Top = 87
      Width = 121
      Height = 58
      BevelOuter = bvLowered
      Caption = 'Abc'
      TabOrder = 5
    end
  end
  object Button1: TButton
    Left = 8
    Top = 44
    Width = 129
    Height = 34
    Caption = 'Browse'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -19
    Font.Name = 'Tahoma'
    Font.Style = []
    Font.Quality = fqClearType
    ParentFont = False
    TabOrder = 1
    OnClick = Button1Click
  end
  object Button2: TButton
    Left = 161
    Top = 296
    Width = 129
    Height = 34
    Caption = 'Save Playlist'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -19
    Font.Name = 'Tahoma'
    Font.Style = []
    Font.Quality = fqClearType
    ParentFont = False
    TabOrder = 2
    OnClick = Button2Click
  end
  object Button3: TButton
    Left = 6
    Top = 296
    Width = 129
    Height = 34
    Caption = 'Load Playlist'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -19
    Font.Name = 'Tahoma'
    Font.Style = []
    Font.Quality = fqClearType
    ParentFont = False
    TabOrder = 3
    OnClick = Button3Click
  end
  object Button4: TButton
    Left = 161
    Top = 44
    Width = 129
    Height = 34
    Caption = 'Update'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -19
    Font.Name = 'Tahoma'
    Font.Style = []
    Font.Quality = fqClearType
    ParentFont = False
    TabOrder = 4
    OnClick = Button4Click
  end
  object UpDown: TUpDown
    Left = 26
    Top = 612
    Width = 17
    Height = 25
    Min = 1
    Max = 10
    TabOrder = 5
    OnChangingEx = UpDownChangingEx
  end
  object GroupBox2: TGroupBox
    Left = 6
    Top = 344
    Width = 284
    Height = 49
    Caption = ' Interface '
    TabOrder = 6
    object CbPanelResize: TCheckBox
      Left = 16
      Top = 22
      Width = 265
      Height = 17
      Caption = ' Panel size proportionnal to window size'
      Checked = True
      State = cbChecked
      TabOrder = 0
      OnClick = CbPanelResizeClick
    end
  end
  object GroupBox3: TGroupBox
    Left = 6
    Top = 558
    Width = 284
    Height = 51
    Caption = 'Begin / End Track Low Volume Cut  '
    TabOrder = 7
    object Label1: TLabel
      Left = 15
      Top = 24
      Width = 47
      Height = 13
      Caption = 'Sensibility'
    end
    object SbSensibility: TScrollBar
      Left = 68
      Top = 21
      Width = 196
      Height = 17
      Max = 20
      PageSize = 0
      Position = 10
      TabOrder = 0
      OnChange = SbSensibilityChange
    end
  end
  object Button5: TButton
    Left = 8
    Top = 100
    Width = 282
    Height = 34
    Caption = 'Verify Valid Audio Files'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -19
    Font.Name = 'Tahoma'
    Font.Style = []
    Font.Quality = fqClearType
    ParentFont = False
    TabOrder = 8
    OnClick = Button5Click
  end
  object MemoInvalidAudioFiles: TMemo
    Left = 8
    Top = 160
    Width = 281
    Height = 101
    TabOrder = 9
  end
  object FileOpenDialog: TFileOpenDialog
    FavoriteLinks = <>
    FileTypes = <>
    Options = [fdoPickFolders]
    Left = 254
    Top = 640
  end
end
