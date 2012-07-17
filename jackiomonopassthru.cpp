// TODO: upp++ says jack_client_open and jack_client_close cause a heap leak

#include "jackiomonopassthru.h"
#include "debug.h"


// private varables

jack_port_t* JackIOmonopassthru::InputPort = 0 ;
jack_port_t* JackIOmonopassthru::OutputPort = 0 ;
jack_client_t* JackIOmonopassthru::Client = 0 ;
Loopidity* JackIOmonopassthru::App = 0 ;


// public functions

int JackIOmonopassthru::Init(Loopidity* app) { App = app ; return Reset() ; }

int JackIOmonopassthru::Reset()
{
	DBG((Client)? "JACK resetting" : "JACK initializing") ;

	const char** ports ;
	const char* client_name = JACK_CLIENT_NAME ;
	const char* server_name = NULL ;
	jack_options_t options = JackNullOption ;
	jack_status_t status ;

	if (Client) Close() ;
	if (!(Client = jack_client_open(client_name , options , &status , server_name)))
		{ DBG("Unable to connect to JACK server") ; return false ; }

	// TODO: must be a JACK1 thing - this is never true
	// DBG((status & JackServerStarted)? "JACK server started" : "JACK server running") ;
	if (status & JackNameNotUnique) client_name = jack_get_client_name(Client) ;
	jack_set_process_callback(Client , Process , 0) ;
	jack_on_shutdown(Client , JackShutdown , 0) ;

	InputPort = jack_port_register(Client , "input" , JACK_DEFAULT_AUDIO_TYPE , JackPortIsInput , 0) ;
	OutputPort = jack_port_register(Client , "output" , JACK_DEFAULT_AUDIO_TYPE , JackPortIsOutput , 0) ;
	if ((InputPort == NULL) || (OutputPort == NULL)) { DBG("no more JACK ports available") ; return false ; }
	if (jack_activate(Client)) { DBG("cannot activate client") ; return false ; }

	ports = jack_get_ports(Client , NULL , NULL , JackPortIsPhysical | JackPortIsOutput) ;
	if (ports == NULL) { DBG("no physical capture ports") ; return false ; }
	if (!jack_connect(Client , ports[0] , jack_port_name(InputPort))) free(ports) ;
	else { DBG("cannot connect input ports") ; return false ; }

	ports = jack_get_ports(Client , NULL , NULL , JackPortIsPhysical | JackPortIsInput) ;
	if (ports == NULL) { DBG("no physical playback ports") ; return false ; }
	if (!jack_connect(Client , jack_port_name(OutputPort) , ports[0])) free(ports) ;
	else { DBG("cannot connect output ports") ; return false ; }

	DBG("JACK initialized") ; return true ;
}

void JackIOmonopassthru::Close()
{
	jack_client_close(Client) ;
	free(InputPort) ;
	free(OutputPort) ;
	free(Client) ;
}


// private functions

int JackIOmonopassthru::Process(jack_nframes_t nframes , void* arg)
{
	jack_default_audio_sample_t* in = (jack_default_audio_sample_t*)jack_port_get_buffer(InputPort , nframes) ;
	jack_default_audio_sample_t* out = (jack_default_audio_sample_t*)jack_port_get_buffer(OutputPort , nframes) ;
	memcpy(out , in , sizeof(jack_default_audio_sample_t) * nframes) ;

	return 0 ;
}

void JackIOmonopassthru::JackShutdown(void* arg) { Close() ; exit(1) ; }
