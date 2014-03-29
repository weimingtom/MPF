#include "stdafx.h"
#include "../include/ui/TextBlock.h"

using namespace MPF;
using namespace MPF::Visual;
using namespace MPF::UI;

DEFINE_TYPE(TextBlock, MPF::UI::TextBlock)
DEFINE_UI_VALUES(TextBlock)
DEFINE_UI_FUNCS(TextBlock, UIElement)

DependencyProperty<std::shared_ptr<MPF::String>> TextBlock::TextProperty(std::make_shared<String>(L"Text"), String::GetEmpty());
DependencyProperty<std::shared_ptr<MPF::Visual::Font>> TextBlock::FontProperty(std::make_shared<String>(L"Font"));

TextBlock::TextBlock()
{
	AddPropertyHandlers();
}

TextBlock::TextBlock(std::shared_ptr<MPF::String> text)
:TextBlock()
{
	Text = text;
}

TextBlock::~TextBlock()
{

}

std::shared_ptr<MPF::String> TextBlock::GetText() const
{
	return GetValue(TextProperty);
}

void TextBlock::SetText(std::shared_ptr<MPF::String> value)
{
	SetValue(TextProperty, value);
}

void TextBlock::OnTextChanged()
{
	//���λ���ʧЧ
	textGlyphs = nullptr;
}

void TextBlock::AddPropertyHandlers()
{
	SetPropertyChangedHandler(TextProperty, [&]
	{
		OnTextChanged(); 
	});
}

void TextBlock::Update(MPF::Visual::RenderCoreProvider& renderer, float elapsedTime)
{
	if (Visibility == Visibility::Visible)
	{
		if (textGlyphs == nullptr)
		{
			UpdateTextGlyphs();
		}
	}
}

void TextBlock::UpdateTextGlyphs()
{
	auto font(Font);
	auto& text(*Text);
	massert(font != nullptr);

	auto size(font->MeasureText(text));
	textGlyphs = std::make_shared<BitmapData<byte>>(size.first, size.second);
	font->DrawText(*textGlyphs, 0, 0, text);
}

std::shared_ptr<MPF::Visual::Font> TextBlock::GetFont() const
{
	return GetValue(FontProperty);
}

void TextBlock::SetFont(std::shared_ptr<MPF::Visual::Font> value)
{
	SetValue(FontProperty, value);
}