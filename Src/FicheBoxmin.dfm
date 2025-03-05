object Boxmin: TBoxmin
  Left = 0
  Top = 0
  BorderIcons = [biSystemMenu]
  BorderWidth = 1
  Caption = '  Boxon Mini'
  ClientHeight = 15
  ClientWidth = 482
  Color = clBtnFace
  TransparentColorValue = clBtnFace
  Constraints.MaxHeight = 30
  Constraints.MinHeight = 30
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWhite
  Font.Height = 8
  Font.Name = 'Tahoma'
  Font.Style = []
  OldCreateOrder = False
  ScreenSnap = True
  OnClose = FormClose
  OnResize = FormResize
  OnShow = FormShow
  PixelsPerInch = 96
  TextHeight = 8
  object Label1: TLabel
    Left = 0
    Top = 0
    Width = 33
    Height = 15
    Align = alLeft
    Caption = 'LbTitre'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clLime
    Font.Height = -11
    Font.Name = 'Tahoma'
    Font.Style = []
    ParentFont = False
    Layout = tlCenter
    StyleElements = [seClient, seBorder]
    OnClick = Label1Click
    ExplicitHeight = 13
  end
end
