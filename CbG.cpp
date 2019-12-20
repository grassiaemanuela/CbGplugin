#include <wx/utils.h>

#include <sdk.h> // Code::Blocks SDK
#include <configurationpanel.h>
#include <logmanager.h>
#include <manager.h>
#include <projectmanager.h>
#include <logger.h>
#include <loggers.h>
#include <cbproject.h>
#include <editormanager.h>
#include <cbeditor.h>
#include <cbstyledtextctrl.h>
#include "CbG.h"

#include "CloneDialog.h"

// Register the plugin with Code::Blocks.
// We are using an anonymous namespace so we don't litter the global one.
namespace
{
    PluginRegistrant<CbG> reg(_T("CbG"));
}


// events handling
BEGIN_EVENT_TABLE(CbG, cbPlugin)
    // add any events you want to handle here
END_EVENT_TABLE()

// constructor
CbG::CbG()
{
    // Make sure our resources are available.
    // In the generated boilerplate code we have no resources but when
    // we add some, it will be nice that this code is in place already ;)
    if(!Manager::LoadResource(_T("CbG.zip")))
    {
        NotifyMissingFile(_T("CbG.zip"));
    }
}

// destructor
CbG::~CbG()
{
}

void CbG::OnAttach()
{
    // do whatever initialization you need for your plugin
    // NOTE: after this function, the inherited member variable
    // m_IsAttached will be TRUE...
    // You should check for it in other functions, because if it
    // is FALSE, it means that the application did *not* "load"
    // (see: does not need) this plugin...


    git = _T("git");
    Logger *gitBlocksLogger = new TextCtrlLogger();
    //a logger which prints messages to a wxTextCtrl
    logSlot = Manager::Get()->GetLogManager()->SetLog(gitBlocksLogger);
    //LogManager è responsabile sia dell'output normale sia dell'output di debug (consultare logmanager.h per maggiori dettagli)
    //Questo nuovo metodo utilizza il metodo Get statico del Manager per restituire l'oggetto Manager singleton, quindi lo utilizza
    //per accedere a LogManager tramite il metodo GetLogManager. LogManager ha un metodo chiamato Log che aggiunge
    //una stringa in fondo al log di output
    //set log restituise un logmanager e richiedere il logger come parametro

    Manager::Get()->GetLogManager()->Slot(logSlot).title = _T("CbG");
    CodeBlocksLogEvent evtAdd1(cbEVT_ADD_LOG_WINDOW, gitBlocksLogger, Manager::Get()->GetLogManager()->Slot(logSlot).title);
    //cbEVT_ADD_LOG_WINDOW aggiunge una finesta wx
    Manager::Get()->ProcessEvent(evtAdd1);
}

void CbG::OnRelease(bool appShutDown)
{
    // do de-initialization for your plugin
    // if appShutDown is true, the plugin is unloaded because Code::Blocks is being shut down,
    // which means you must not use any of the SDK Managers
    // NOTE: after this function, the inherited member variable
    // m_IsAttached will be FALSE...

    Manager::Get()->GetLogManager()->DeleteLog(logSlot);
}


void CbG::RegisterFunction(wxObjectEventFunction func, wxString label)
{
    wxMenuItem *item = new wxMenuItem(menu, wxID_ANY, label, label);
    menu->Append(item);
    Connect(item->GetId(), wxEVT_COMMAND_MENU_SELECTED, func);
}

void CbG::BuildMenu(wxMenuBar* menuBar)
{
    //The application is offering its menubar for your plugin,
    //to add any menu items you want...
    //Append any items you need in the menu...
    //NOTE: Be careful in here... The application's menubar is at your disposal.
     menu = new wxMenu();
    //creiamo un menu e per ciascuna voce utilizziamo la funzione registerfunction(evento, commento)
    RegisterFunction(wxCommandEventHandler(CbG::Init), _("Crea un repository vuoto"));
    menu->AppendSeparator();
    RegisterFunction(wxCommandEventHandler(CbG::Clone), _("Scarica traccia"));
    menu->AppendSeparator();
    RegisterFunction(wxCommandEventHandler(CbG::Push), _("Consegna"));

    menuBar->Insert(menuBar->FindMenu(_("&Tools")) + 1, menu, wxT("&Esame"));
}


void CbG::Execute(wxString command, const wxString comment, wxString dir)
{
    if(dir.empty())
        dir = Manager::Get()->GetProjectManager()->GetActiveProject()->GetBasePath();

    wxArrayString output;

    Manager::Get()->GetLogManager()->Log(comment, logSlot);//mostro in log il nostro commento con un intero (logslot)
    Manager::Get()->GetLogManager()->Log(command, logSlot);

    wxString ocwd = wxGetCwd();//acquisisce la directory corrente di lavoro
    wxSetWorkingDirectory(dir);
    wxExecute(command, output);
    wxSetWorkingDirectory(ocwd);

    for(unsigned int i=0; i<output.size(); i++)
        Manager::Get()->GetLogManager()->Log(output[i], logSlot);
}

void CbG::ExecuteInTerminal(wxString command, const wxString comment, wxString dir)
{
#ifdef __WXMSW__ // Windows needs some extra code
	wxString newcmd = _T("cmd.exe /C \"") + command + _T("\"");
#else
	wxString newcmd = _T( "xterm -e \"" ) + command + _T("\"");
#endif
	Execute(newcmd, comment, dir);
}


void CbG::Init(wxCommandEvent &event)
{
    Execute(git + _T(" init"), _("Creating an empty git repository ..."));//comando e commento
}
void CbG::Clone(wxCommandEvent &event)
{
	CloneDialog dialog(Manager::Get()->GetAppWindow());
	if(dialog.ShowModal() == wxID_OK) //mostriamo la finestra in modo modale e vediamo se ha cliccato il pulsante ok
	{
		wxString command = git + _T(" clone ") + dialog.Origin->GetValue(); //si prende il valore origin dalla finestra clonedialog
		ExecuteInTerminal(command, _("Cloning repository ..."), dialog.Directory->GetValue());
		//esegue nel terminale l'struzione e come commento utilizziamo cloningrepository
	}

	//Execute(_T("vim .gitignore"), _("aggiunta del file .gitignore"));
}

void CbG::Push(wxCommandEvent &event)
{
	ExecuteInTerminal(git + _T(" push origin HEAD"), _("Pushing HEAD to origin ..."));
	//se voglio fare il push il comando è semplicemente "git push origin head"
}
