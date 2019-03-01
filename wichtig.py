import sys,os,subprocess

def warn( text ):
    '''Gibt Argument mit auf stderr aus'''
    print >>sys.stderr, text

def die( text ):
    '''Gibt Argument mit auf stderr aus, und beendet das Programm mit Status = 1'''
    print >>sys.stderr, text
    sys.exit(1)

def kommando2( aufrufliste, ein=None, ok_liste=[0] ):
    '''
    Fuhrt Kommando mit stdin, stdout und stderr geknuepft 
    mit dem ausfuehrenden Python Code
    myausgabe, myfehlerausgabe = kommando( mybefehl, myeingabe )
    Bsp: 
    liste = 'gorgonzola cheese\nchianti\noliven\n'
    aus, err = kommando( 'gegrep cheese', liste )
    '''
        
    def rufe_befehl_auf( liste, ein ):
        procobj = subprocess.Popen(
                liste,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                stdin=subprocess.PIPE )
        aus,err = procobj.communicate(ein)
        stat = procobj.returncode
        if stat not in ok_liste:
            raise Exception,'''
            kommando: ABBRUCH Programm fehlgeschlagen:
            %s
            Status:%s
            Fehlermeldung:%s'''%(','.join(liste),stat,err)
        return aus, err

    liste = list()
    gesamt_err = list()
    for el in aufrufliste:
        if el == '|':
            # Pipe 
            aus, err = rufe_befehl_auf( liste, ein )
            liste = list()
            ein = aus
            gesamt_err.append(err)
        else:
            liste.append(el)
    aus, err = rufe_befehl_auf( liste, ein )
    gesamt_err.append(err)
    return aus, '\n'.join( gesamt_err )

def kommando( was, ein=None, ok_liste=[0] ):
    '''
    Fuhrt Kommando mit stdin, stdout und stderr geknuepft 
    mit dem ausfuehrenden Python Code
    myausgabe, myfehlerausgabe = kommando( mybefehl, myeingabe )
    Bsp: 
    liste = 'gorgonzola cheese\nchianti\noliven\n'
    aus, err = kommando( 'gegrep cheese', liste )
    '''
    procobj = subprocess.Popen(
            was.split(),
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            stdin=subprocess.PIPE )
    aus,err = procobj.communicate(ein)
    stat = procobj.returncode
    if stat not in ok_liste:
        raise Exception,'''
        kommando: ABBRUCH Programm fehlgeschlagen:
        %s
        Status:%s
        Fehlermeldung:%s'''%(was,stat,err)
    return aus, err

def tee( befehl, logdatei, abschiedsgruss='default_abschiedsgruss_soll_nie_vorkommen' ):
    logfh = file( logdatei, 'w' )
    popen = subprocess.Popen(
        befehl.split(),
        stderr=subprocess.STDOUT,
        stdout=subprocess.PIPE )

    aus = list()
    poll = None
    zeile = ''
    while abschiedsgruss not in zeile and poll == None:
        zeile = popen.stdout.readline()
        print zeile,
        logfh.write( zeile )
        aus.append( zeile )
        poll = popen.poll()

    logfh.close()
    return aus

