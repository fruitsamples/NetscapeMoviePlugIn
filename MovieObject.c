/*
 IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. ("Apple") in
 consideration of your agreement to the following terms, and your use, installation, 
 modification or redistribution of this Apple software constitutes acceptance of these 
 terms.  If you do not agree with these terms, please do not use, install, modify or 
 redistribute this Apple software.
 
 In consideration of your agreement to abide by the following terms, and subject to these 
 terms, Apple grants you a personal, non-exclusive license, under Apple�s copyrights in 
 this original Apple software (the "Apple Software"), to use, reproduce, modify and 
 redistribute the Apple Software, with or without modifications, in source and/or binary 
 forms; provided that if you redistribute the Apple Software in its entirety and without 
 modifications, you must retain this notice and the following text and disclaimers in all 
 such redistributions of the Apple Software.  Neither the name, trademarks, service marks 
 or logos of Apple Computer, Inc. may be used to endorse or promote products derived from 
 the Apple Software without specific prior written permission from Apple. Except as expressly
 stated in this notice, no other rights or licenses, express or implied, are granted by Apple
 herein, including but not limited to any patent rights that may be infringed by your 
 derivative works or by other works in which the Apple Software may be incorporated.
 
 The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO WARRANTIES, 
 EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, 
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS 
 USE AND OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
 
 IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL 
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS 
 OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, 
 REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND 
 WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR 
 OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#import "MovieObject.h"

#import "movie.h"

static void movieInvalidate(NPObject *obj);
static bool movieHasProperty(NPObject *obj, NPIdentifier name);
static bool movieHasMethod(NPObject *obj, NPIdentifier name);
static bool movieGetProperty(NPObject *obj, NPIdentifier name, NPVariant *variant);
static bool movieSetProperty(NPObject *obj, NPIdentifier name, const NPVariant *variant);
static bool movieInvoke(NPObject *obj, NPIdentifier name, const NPVariant *args, uint32_t argCount, NPVariant *result);
static bool movieInvokeDefault(NPObject *obj, const NPVariant *args, uint32_t argCount, NPVariant *result);
static NPObject *movieAllocate(NPP npp, NPClass *theClass);
static void movieDeallocate(NPObject *obj);

static NPClass movieClass = { 
    NP_CLASS_STRUCT_VERSION,
    movieAllocate, 
    movieDeallocate, 
    movieInvalidate,
    movieHasMethod,
    movieInvoke,
    movieInvokeDefault,
    movieHasProperty,
    movieGetProperty,
    movieSetProperty,
};

NPClass *getMovieClass(void)
{
    return &movieClass;
}

static bool identifiersInitialized = false;

#define ID_MUTED_PROPERTY               0
#define NUM_PROPERTY_IDENTIFIERS        1

static NPIdentifier moviePropertyIdentifiers[NUM_PROPERTY_IDENTIFIERS];
static const NPUTF8 *moviePropertyIdentifierNames[NUM_PROPERTY_IDENTIFIERS] = {
    "muted"
};

#define ID_PLAY_METHOD                          0
#define ID_PAUSE_METHOD                         1
#define NUM_METHOD_IDENTIFIERS                  2

static NPIdentifier movieMethodIdentifiers[NUM_METHOD_IDENTIFIERS];
static const NPUTF8 *movieMethodIdentifierNames[NUM_METHOD_IDENTIFIERS] = {
    "play",
    "pause"
};

static void initializeIdentifiers(void)
{
    browser->getstringidentifiers(moviePropertyIdentifierNames, NUM_PROPERTY_IDENTIFIERS, moviePropertyIdentifiers);
    browser->getstringidentifiers(movieMethodIdentifierNames, NUM_METHOD_IDENTIFIERS, movieMethodIdentifiers);
}

bool movieHasProperty(NPObject *obj, NPIdentifier name)
{
    int i;
    for (i = 0; i < NUM_PROPERTY_IDENTIFIERS; i++)
        if (name == moviePropertyIdentifiers[i])
            return true;
    return false;
}

bool movieHasMethod(NPObject *obj, NPIdentifier name)
{
    int i;
    for (i = 0; i < NUM_METHOD_IDENTIFIERS; i++)
        if (name == movieMethodIdentifiers[i])
            return true;
    return false;
}

bool movieGetProperty(NPObject *obj, NPIdentifier name, NPVariant *variant)
{
    MovieObject *movieObj = (MovieObject *)obj;
    if (name == moviePropertyIdentifiers[ID_MUTED_PROPERTY]) {
        BOOLEAN_TO_NPVARIANT(IsMovieMuted(movieObj), *variant);
        return true;
    }
    return false;
}

bool movieSetProperty(NPObject *obj, NPIdentifier name, const NPVariant *variant)
{
    MovieObject *movieObj = (MovieObject *)obj;
    if (name == moviePropertyIdentifiers[ID_MUTED_PROPERTY]) {
        if (NPVARIANT_IS_BOOLEAN(*variant)) {
            SetMovieMuted(movieObj, NPVARIANT_TO_BOOLEAN(*variant));
            return true;
        }
        return false;
    }
    return false;
}

bool movieInvoke(NPObject *obj, NPIdentifier name, const NPVariant *args, uint32_t argCount, NPVariant *result)
{
    MovieObject *movieObj = (MovieObject *)obj;
    if (name == movieMethodIdentifiers[ID_PLAY_METHOD]) {
        PlayMovie(movieObj);
        VOID_TO_NPVARIANT(*result);
        return true;
    }
    if (name == movieMethodIdentifiers[ID_PAUSE_METHOD]) {
        PauseMovie(movieObj);
        VOID_TO_NPVARIANT(*result);
        return true;
    }
    return false;
}

bool movieInvokeDefault(NPObject *obj, const NPVariant *args, uint32_t argCount, NPVariant *result)
{
    MovieObject *movieObj = (MovieObject *)obj;
    if (argCount == 1) {
        if (NPVARIANT_IS_STRING(args[0])) {
            NPString string = NPVARIANT_TO_STRING(args[0]);
            if (strncmp(string.UTF8Characters, "play", string.UTF8Length) == 0) {
                PlayMovie(movieObj);
                VOID_TO_NPVARIANT(*result);
                return true;
            }
            if (strncmp(string.UTF8Characters, "pause", string.UTF8Length) == 0) {
                PauseMovie(movieObj);
                VOID_TO_NPVARIANT(*result);
                return true;
            }
        }
    }
    
    return false;
}

void movieInvalidate(NPObject *obj)
{
    // Release any remaining references to JavaScript objects.
}

NPObject *movieAllocate(NPP npp, NPClass *theClass)
{
    MovieObject *newInstance = malloc(sizeof(MovieObject));
    
    if (!identifiersInitialized) {
        identifiersInitialized = true;
        initializeIdentifiers();
    }
    newInstance->movie = 0;
    newInstance->controller = 0;
    
    return &newInstance->header;
}

void movieDeallocate(NPObject *obj) 
{
    free(obj);
}

// ------ Actual implementation of plugin movie playing functionality.

bool LoadMovieFromFile(Movie *movie, const char *fname)
{    
    OSErr err;
    FSSpec spec;
    
    if (fname[0] == '/') {
        // Browser handed us a POSIX path.
        FSRef fref;
        err = FSPathMakeRef((const UInt8 *)fname, &fref, NULL);
        if (err != noErr) {
            printf("FSPathMakeRef failed %d\n", err);
            return false;
        }
        err = FSGetCatalogInfo(&fref, kFSCatInfoNone, NULL, NULL, &spec, NULL);
        if (err != noErr) {
            printf("FSGetCatalogInfo failed %d\n", err);
            return false;
        }
    } else {
        Str255 path;
        CopyCStringToPascal(fname, path);
        // Browser handed us a classic path.
        err = FSMakeFSSpec(0, 0, path, &spec);
        if (err != noErr) {
            printf("FSMakeFSSpec failed %d\n", err);
            return false;
        }
    }
    
    short movieResFile = 0;
    err = OpenMovieFile(&spec, &movieResFile, fsRdPerm);
    if (err != noErr) {
        printf("OpenMovieFile failed %d\n", err);
        return false;
    }
    
    Str255 name;
    err = NewMovieFromFile(movie, movieResFile, NULL, name, 0, NULL);
    if (err != noErr) {        
        printf("NewMovieFromFile failed %d\n", err);
        err = CloseMovieFile(movieResFile);
        if (err != noErr) {
            printf("CloseMovieFile failed %d\n", err);
        }
        return false;
    }
    
    err = CloseMovieFile(movieResFile);
    if (err != noErr) {
        printf("CloseMovieFile failed %d\n", err);
    }
    
    return true;
}

bool CreateMovieController(MovieObject *obj, NPWindow *window)
{
    if (obj->movie == NULL) {
        return false;
    }
    
    CGrafPtr port = ((NP_Port *)window->window)->port;
    WindowRef w = GetWindowFromPort(port);
    
    SetMovieGWorld(obj->movie, port, NULL);
    
    Rect rect = {0, 0, 0, 0};
    obj->controller = NewMovieController(obj->movie, &rect, mcScaleMovieToFit);
    MCSetControllerPort(obj->controller, port);
    MCDoAction(obj->controller, mcActionSetKeysEnabled, (void*)((int)true));
    MCDoAction(obj->controller, mcActionSetDragEnabled, (void*)((int)false));
    MCActivate(obj->controller, w, IsWindowHilited(w));
    MCSetControllerAttached(obj->controller, true);
    
    UpdateMovieFrame(obj, window);
    
    return true;
}


#define kMovieControllerHeight 16

void UpdateMovieFrame(MovieObject *obj, NPWindow *window) 
{   
    if (obj->movie != NULL && obj->controller != NULL) {
        Rect rect;
        
        SetRect(&rect, 0, 0, window->width, window->height);
        MCSetControllerBoundsRect(obj->controller, &rect);
        
        rect.bottom -= kMovieControllerHeight;
        SetMovieBox(obj->movie, &rect);
    }
}

void PlayMovie(MovieObject *obj)
{
    if (obj->movie != NULL)
        StartMovie(obj->movie);
}

void PauseMovie(MovieObject *obj)
{
    if (obj->movie != NULL)
        StopMovie(obj->movie);
}

bool IsMovieMuted(MovieObject *obj)
{
    return GetMovieVolume(obj->movie) < 0;
}

void SetMovieMuted(MovieObject *obj, bool mute)
{
    short vol = GetMovieVolume(obj->movie);
    if ((mute && vol > 0) || (!mute && vol < 0))
        SetMovieVolume(obj->movie, -vol);
}

void DestroyMovie(MovieObject *obj)
{
    PauseMovie(obj);
    if (obj->controller != NULL) {
        DisposeMovieController(obj->controller);
        obj->controller = NULL;
    }
    if (obj->movie != NULL) {
        DisposeMovie(obj->movie);
        obj->movie = NULL;
    }
}

bool HandleMovieEvent(MovieObject *obj, EventRecord *event)
{
    if (obj->controller)
        MCIsPlayerEvent(obj->controller, event);
    return false;
}
