/*
 *  wxXMLSupport.cpp
 *
 *  Created by Daniel Swain on 10/30/09.
 *
 */

#include "wxXMLSupport.h"

#include <algorithm> // for find

#include <wx/stdpaths.h>  // used in MT_GetXMLPath
#include <wx/filename.h>  // used in MT_GetXMLPath

#include "MT/MT_GUI/support/wxSupport.h"

wxString MT_GetXMLPathForRobots()
{
    wxString NormalPath = MT_GetXMLPath(wxT(""));
    wxString path = NormalPath.BeforeLast(wxFileName::GetPathSeparator()) + 
        wxFileName::GetPathSeparator() +
        wxT("MADTraC Robots");                                 
  
    return path;
}

wxString MT_GetXMLRootName()
{
    wxString root = wxT("MADTraC_Config_") + wxTheApp->GetAppName();
    return MT_ReplaceSpaces(root,"_");
}

wxString MT_GetXMLPathForApp()
{
  
    wxString filename = wxTheApp->GetAppName() + wxT(".xml");
  
    return MT_GetXMLPath(filename);
  
}

wxString MT_GetXMLPath(const wxString& filename)
{
    wxString path = wxT("");
  
#ifdef __APPLE__
    // CF for Core Foundation
    wxStandardPathsCF sp;
#else
    wxStandardPaths sp;
#endif
    path = sp.GetUserDataDir();
    if(!wxFileName::IsDirWritable(path))
    {
        printf("XML Path %s does not exist, creating it.\n", (const char*) path.c_str());
        if(!wxFileName::Mkdir(path, 0755))
        {
            return filename;
        }
    }
                                 
    if(filename != wxEmptyString)
    {
        path = path + wxFileName::GetPathSeparator() + filename;
    }
  
    return path;
  
}

void MT_ReadWindowDataFromXML(const MT_XMLFile& xmlfile, const wxString& Key, wxWindow* window)
{
  
    if(!window)
    {
        return;
    }
  
    wxString NodeName = wxT("Window_") + MT_ReplaceSpaces(Key,"_");
    wxString rKey, rText;
    wxSize rSize = window->GetSize();
    wxPoint rPosition = window->GetPosition();
    long tmp;
    TiXmlElement* pElem = xmlfile.FirstChild((const char*) NodeName.mb_str()).FirstChild().Element();
    for(/* pElem already initialized */; pElem; pElem = pElem->NextSiblingElement())
    {
        const char* pKey = pElem->Value();
        const char* pText = pElem->GetText();
        if(pKey && pText)
        {
            rKey = MT_StringToWxString(std::string(pKey));;
            rText = MT_StringToWxString(std::string(pText));
            if(!rText.ToLong(&tmp))
            {
                continue;
            }
            if(rKey == wxT("XPos"))
            {
                rPosition.x = tmp;
            }
      
            if(rKey == wxT("YPos"))
            {
                rPosition.y = tmp;
            }

            if(rKey == wxT("Width"))
            {
                rSize.x = tmp;
            }

            if(rKey == wxT("Height"))
            {
                rSize.y = tmp;
            }

        }
    }
  
    if(window->GetWindowStyle() & wxRESIZE_BORDER)
    {
        window->SetSize(rSize);
    }

    /* make sure the window isn't off the screen */
    if(rPosition.x < 0.95*wxSystemSettings::GetMetric(wxSYS_SCREEN_X) &&
       rPosition.y < 0.95*wxSystemSettings::GetMetric(wxSYS_SCREEN_Y))
    {
        window->Move(rPosition);
    }
  
}

void MT_WriteWindowDataToXML(MT_XMLFile* xmlfile, const wxString& Key, wxWindow* window)
{
  
    if(!window || !xmlfile)
    {
        return;
    }
    
    wxString NodeName = wxT("Window_") + MT_ReplaceSpaces(Key,"_");
    TiXmlElement* msgs = xmlfile->FirstChild((const char*) NodeName.mb_str()).Element();
    if(!msgs)
    {
        msgs = new TiXmlElement((const char*) NodeName.mb_str());
        TiXmlElement* root = xmlfile->RootAsElement();
        if(!root)
        {
            delete msgs;
            return;
        }
        root->LinkEndChild(msgs);
    }
    wxString val;
  
    wxPoint cPosition = window->GetPosition();
    val.Printf(wxT("%d"), cPosition.x);
    MT_AddOrReplaceNodeValue(msgs, "XPos", (const char*) val.mb_str());
    val.Printf(wxT("%d"), cPosition.y);
    MT_AddOrReplaceNodeValue(msgs, "YPos", (const char*) val.mb_str());

    if(window->GetWindowStyle() & wxRESIZE_BORDER)
    {
        wxSize cSize = window->GetSize();
        val.Printf(wxT("%d"), cSize.x);
        MT_AddOrReplaceNodeValue(msgs, "Width", (const char*) val.mb_str());
        val.Printf(wxT("%d"), cSize.y);
        MT_AddOrReplaceNodeValue(msgs, "Height", (const char*) val.mb_str());
    }
  
}

MT_PathGroup::MT_PathGroup(const wxString& groupname)
{
  
    m_sGroupName = groupname;
    m_vsNames.resize(0);
    m_vpsPaths.resize(0);
  
}

wxString MT_PathGroup::GetGroupName() const
{
    return m_sGroupName;
}

int MT_PathGroup::GetPathIndexByName(const wxString& name) const
{
  
    for(unsigned int i = 0; i < m_vsNames.size(); i++)
    {
        if(m_vsNames[i] == name)
        {
            return i;
        }
    }
  
    return -1;
  
}

bool MT_PathGroup::HavePath(const wxString& name) const
{
  
    return (GetPathIndexByName(name) >= 0);
  
}

void MT_PathGroup::AddPath(const wxString& name, wxString* path)
{  
    if(!HavePath(name))
    {
        m_vsNames.push_back(name);
        m_vpsPaths.push_back(path);
    }
  
}

wxString MT_PathGroup::GetPathByName(const wxString& name) const
{
    int i = GetPathIndexByName(name);

    if(i < 0)  /* not found */
    {
        return wxT("error");
    }
    else
    {
        return *(m_vpsPaths[i]);
    }
}

void MT_PathGroup::SetPathByName(const wxString& name, const wxString& path)
{
    int i = GetPathIndexByName(name);
  
    if(i < 0)  /* not found */
    {
        return;
    }
    else
    {
        *(m_vpsPaths[i]) = path;
    }
}

void MT_PathGroup::ReadFromXML(const MT_XMLFile& xmlfile)
{  
  
    /* found -> extract paths */
    wxString NodeName = wxT("Path_Group_") + MT_ReplaceSpaces(m_sGroupName,"_");
    TiXmlElement* pElem = xmlfile.FirstChild((const char*) NodeName.mb_str()).FirstChild().Element();
    for(/* pElem already initialized */; pElem; pElem=pElem->NextSiblingElement())
    {
        const char *pKey=pElem->Value();
        const char *pText=pElem->GetText();
        if (pKey && pText) 
        {
            SetPathByName(MT_StringToWxString(std::string(pKey)),
                          MT_StringToWxString(std::string(pText)));
        }
    }
  
}

void MT_PathGroup::WriteToXML(MT_XMLFile* xmlfile)
{
  
    wxString NodeName = wxT("Path_Group_") + MT_ReplaceSpaces(m_sGroupName,"_");
    TiXmlElement* msgs = xmlfile->FirstChild((const char*) NodeName.mb_str()).Element();
    if(!msgs)
    {
        msgs = new TiXmlElement(NodeName.mb_str());
        xmlfile->RootAsElement()->LinkEndChild(msgs);
    }
    TiXmlNode* msg;
    TiXmlNode* old_msg;
    for(unsigned int i = 0; i < m_vsNames.size(); i++)
    {
        wxString curr_name = MT_ReplaceSpaces(m_vsNames[i],"_");
        old_msg = msgs->FirstChild((const char*) curr_name.mb_str());
        msg = new TiXmlElement((const char*) curr_name.mb_str());
        msg->LinkEndChild(new TiXmlText((const char*) (*(m_vpsPaths[i])).mb_str()));
        if(old_msg)
        {
            msgs->ReplaceChild(old_msg, *msg);
            /* ReplaceChild copies msg, so we need to delete ours */
            delete msg;
        }
        else
        {
            msgs->LinkEndChild( msg );  
            /* LinkEndChild does NOT copy msg -> the document now owns it,
            * so we should NOT delete it */
        }
    }
}
