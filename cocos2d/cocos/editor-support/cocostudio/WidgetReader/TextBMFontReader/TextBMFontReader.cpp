/****************************************************************************
 Copyright (c) 2017-2018 Xiamen Yaji Software Co., Ltd.
 
 http://www.cocos2d-x.org
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/



#include "editor-support/cocostudio/WidgetReader/TextBMFontReader/TextBMFontReader.h"

#include "2d/CCFontAtlasCache.h"
#include "ui/UITextBMFont.h"
#include "platform/CCFileUtils.h"
#include "editor-support/cocostudio/CocoLoader.h"
#include "editor-support/cocostudio/CSParseBinary_generated.h"
#include "editor-support/cocostudio/LocalizationManager.h"

#include "tinyxml2.h"
#include "flatbuffers/flatbuffers.h"

USING_NS_CC;
using namespace ui;
using namespace flatbuffers;

namespace cocostudio
{
static const char* P_FileNameData = "fileNameData";
static const char* P_Text = "text";

static TextBMFontReader* instanceTextBMFontReader = nullptr;

IMPLEMENT_CLASS_NODE_READER_INFO(TextBMFontReader)

TextBMFontReader::TextBMFontReader()
{

}

TextBMFontReader::~TextBMFontReader()
{

}

TextBMFontReader* TextBMFontReader::getInstance()
{
    if (!instanceTextBMFontReader)
    {
        instanceTextBMFontReader = new(std::nothrow) TextBMFontReader();
    }
    return instanceTextBMFontReader;
}

void TextBMFontReader::destroyInstance()
{
    CC_SAFE_DELETE(instanceTextBMFontReader);
}

void TextBMFontReader::setPropsFromBinary(cocos2d::ui::Widget* widget, CocoLoader* cocoLoader, stExpCocoNode* cocoNode)
{
    this->beginSetBasicProperties(widget);

    TextBMFont* labelBMFont = static_cast<TextBMFont*>(widget);

    stExpCocoNode* stChildArray = cocoNode->GetChildArray(cocoLoader);

    for (int i = 0; i < cocoNode->GetChildNum(); ++i)
    {
        std::string key = stChildArray[i].GetName(cocoLoader);
        std::string value = stChildArray[i].GetValue(cocoLoader);
        //read all basic properties of widget
        CC_BASIC_PROPERTY_BINARY_READER
            //read all color related properties of widget
        CC_COLOR_PROPERTY_BINARY_READER

        else if (key == P_FileNameData)
        {
            stExpCocoNode* backGroundChildren = stChildArray[i].GetChildArray(cocoLoader);
            std::string resType = backGroundChildren[2].GetValue(cocoLoader);

            Widget::TextureResType imageFileNameType = (Widget::TextureResType)valueToInt(resType);

            std::string backgroundValue = this->getResourcePath(cocoLoader, &stChildArray[i], imageFileNameType);
            if (imageFileNameType == (Widget::TextureResType)0)
            {
                labelBMFont->setFntFile(backgroundValue);
            }

        }
        else if (key == P_Text)
        {
            labelBMFont->setString(value);
        }
    } //end of for loop
    this->endSetBasicProperties(widget);
}

void TextBMFontReader::setPropsFromJsonDictionary(Widget* widget, const rapidjson::Value &options)
{
    WidgetReader::setPropsFromJsonDictionary(widget, options);

    std::string jsonPath = GUIReader::getInstance()->getFilePath();

    TextBMFont* labelBMFont = static_cast<TextBMFont*>(widget);

    const rapidjson::Value &cmftDic = DICTOOL->getSubDictionary_json(options, P_FileNameData);
    int cmfType = DICTOOL->getIntValue_json(cmftDic, P_ResourceType);
    switch (cmfType)
    {
        case 0:
        {
            std::string tp_c = jsonPath;
            const char* cmfPath = DICTOOL->getStringValue_json(cmftDic, P_Path);
            const char* cmf_tp = tp_c.append(cmfPath).c_str();
            labelBMFont->setFntFile(cmf_tp);
            break;
        }
        case 1:
            CCLOG("Wrong res type of LabelAtlas!");
            break;
        default:
            break;
    }

    const char* text = DICTOOL->getStringValue_json(options, P_Text, "Text Label");
    labelBMFont->setString(text);

    WidgetReader::setColorPropsFromJsonDictionary(widget, options);
}

Offset<Table> TextBMFontReader::createOptionsWithFlatBuffers(const tinyxml2::XMLElement* objectData,
                                                             flatbuffers::FlatBufferBuilder* builder)
{
    auto temp = WidgetReader::getInstance()->createOptionsWithFlatBuffers(objectData, builder);
    auto widgetOptions = *(Offset<WidgetOptions>*)(&temp);

    std::string text = "Fnt Text Label";
    bool isLocalized = false;

    std::string path = "";
    std::string plistFlie = "";
    int resourceType = 0;

    // attributes
    const tinyxml2::XMLAttribute* attribute = objectData->FirstAttribute();
    while (attribute)
    {
        std::string name = attribute->Name();
        std::string value = attribute->Value();

        if (name == "LabelText")
        {
            text = value;
        }
        else if (name == "IsLocalized")
        {
            isLocalized = (value == "True") ? true : false;
        }

        attribute = attribute->Next();
    }

    // child elements
    const tinyxml2::XMLElement* child = objectData->FirstChildElement();
    while (child)
    {
        std::string name = child->Name();

        if (name == "LabelBMFontFile_CNB")
        {
            attribute = child->FirstAttribute();

            while (attribute)
            {
                name = attribute->Name();
                std::string value = attribute->Value();

                if (name == "Path")
                {
                    path = value;
                }
                else if (name == "Type")
                {
                    resourceType = 0;
                }
                else if (name == "Plist")
                {
                    plistFlie = value;
                }

                attribute = attribute->Next();
            }
        }

        child = child->NextSiblingElement();
    }

    auto options = CreateTextBMFontOptions(*builder, widgetOptions,
                                           CreateResourceData(*builder, builder->CreateString(path),
                                                              builder->CreateString(plistFlie), resourceType),
                                           builder->CreateString(text), isLocalized);

    return *(Offset<Table>*)(&options);
}

void TextBMFontReader::setPropsWithFlatBuffers(cocos2d::Node* node, const flatbuffers::Table* textBMFontOptions)
{
    TextBMFont* labelBMFont = static_cast<TextBMFont*>(node);
    auto options = (TextBMFontOptions*)textBMFontOptions;

    auto cmftDic = options->fileNameData();
    bool fileExist = false;
    std::string errorFilePath = "";
    std::string errorContent = "";
    std::string path = cmftDic->path()->c_str();
    int cmfType = cmftDic->resourceType();
    switch (cmfType)
    {
        case 0:
        {
            if (FileUtils::getInstance()->isFileExist(path))
            {
                FontAtlas* newAtlas = FontAtlasCache::getFontAtlasFNT(path);
                if (newAtlas)
                {
                    fileExist = true;
                }
                else
                {
                    errorContent = "has problem";
                    fileExist = false;
                }
            }
            break;
        }

        default:
            break;
    }
    if (fileExist)
    {
        labelBMFont->setFntFile(path);
    }

    std::string text = options->text()->c_str();
    bool isLocalized = options->isLocalized() != 0;
    if (isLocalized)
    {
        ILocalizationManager* lm = LocalizationHelper::getCurrentManager();
        labelBMFont->setString(lm->getLocalizationString(text));
    }
    else
    {
        labelBMFont->setString(text);
    }

    auto widgetReader = WidgetReader::getInstance();
    widgetReader->setPropsWithFlatBuffers(node, (Table*)options->widgetOptions());

    labelBMFont->ignoreContentAdaptWithSize(true);
}

Node* TextBMFontReader::createNodeWithFlatBuffers(const flatbuffers::Table* textBMFontOptions)
{
    TextBMFont* textBMFont = TextBMFont::create();

    setPropsWithFlatBuffers(textBMFont, (Table*)textBMFontOptions);

    return textBMFont;
}

}
