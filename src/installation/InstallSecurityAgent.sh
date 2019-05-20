#!/bin/bash

_userName="asciotagent"
_serviceTemplateName="ASCIoTAgent.service"
_systemServiceFileLocation="/etc/systemd/system"
_targetDirectory="/var/ASCIoTAgent"
_execName="ASCIoTAgent"
_scriptDir=
_mode="none"
_omsBaselineExecutablePath="https://raw.githubusercontent.com/Microsoft/OMS-Agent-for-Linux/master/source/code/plugins/"
_omsBaselineX64="omsbaseline_x64"
_omsBaselineX86="omsbaseline_x86"
_omsAudits="oms_audits.xml"
_omsbaselineExe="omsbaseline"
_identity=
_authenticationMethod=
_filePath=
_deviceId=
_hostName=

usage()
{
    echo "usage: InstallSecurityAgent [[-aui <authentication identity> ] [[-aum <authentication method> ] [[-f <file path> ] [[-hn <host name> ] [[-di <device id> ] [-i | -u] | [-h]]"
}

wgetAndExitOnFail(){
    wget "$1" -O "$2"
    if [ $? -ne 0 ]; then
        echo "Couldn't donwload $1, exiting...";
        exit 1;
    fi
}

uninstallagent()
{
    echo uninstalling agent

    #stop the daemon
    systemctl stop $_serviceTemplateName

    #disable the service
    systemctl disable $_serviceTemplateName

    #remove the agent files
    rm -rf $_targetDirectory 

    #remove the service user
    deluser --remove-home --remove-all-files $_userName

    #remove the service configuration
    rm $_systemServiceFileLocation/$_serviceTemplateName

    echo agent uninstallation finished
}

installagent()
{
    #install dependencies
    echo installing agent dependencies
    apt-get install -y auditd
    apt-get install -y uuid-runtime
    
    echo installing agent
    
    #add the service user
    echo creating service user
    adduser --disabled-login --gecos 'ASC IoT Agent' $_userName
    usermod -a -G sudo $_userName

    #get oms executables:
    wgetAndExitOnFail "${_omsBaselineExecutablePath}${_omsAudits}" "$_scriptDir/$_omsAudits"

    if [ $(uname -m) == 'x86_64' ]; then
        wgetAndExitOnFail "${_omsBaselineExecutablePath}${_omsBaselineX64}"  "$_scriptDir/$_omsbaselineExe"	
    elif [ $(uname -m) == 'x86' ]; then
        wgetAndExitOnFail "${_omsBaselineExecutablePath}${_omsBaselineX86}" "$_scriptDir/$_omsbaselineExe"
    fi

    #populate the target directory with the agent files 
    mkdir $_targetDirectory
    cp -r $_scriptDir/* $_targetDirectory
    
    #add connection string in app.config
    sed -i -e "s|\"Identity\" : \"[^\"]*\"|\"Identity\" : \"$_identity\"|g" $_targetDirectory/LocalConfiguration.json
    sed -i -e "s|\"AuthenticationMethod\" : \"[^\"]*\"|\"AuthenticationMethod\" : \"$_authenticationMethod\"|g" $_targetDirectory/LocalConfiguration.json
    sed -i -e "s|\"FilePath\" : \"[^\"]*\"|\"FilePath\" : \"$_filePath\"|g" $_targetDirectory/LocalConfiguration.json
    sed -i -e "s|\"DeviceId\" : \"[^\"]*\"|\"DeviceId\" : \"$_deviceId\"|g" $_targetDirectory/LocalConfiguration.json
    sed -i -e "s|\"HostName\" : \"[^\"]*\"|\"HostName\" : \"$_hostName\"|g" $_targetDirectory/LocalConfiguration.json

    #generate agentId GUID
    agentId=$(uuidgen)
    sed -i -e "s|\"AgentId\" : \"[^\"]*\"|\"AgentId\" : \"$agentId\"|g" $_targetDirectory/LocalConfiguration.json
    
    #make the agent user owner of the target directory
    chown -R $_userName:$_userName $_targetDirectory
    
    #make agent executable
    chown root $_targetDirectory/$_execName
    chmod 4755 $_targetDirectory/$_execName
    chmod 4755 $_targetDirectory/$_omsbaselineExe
    
    #replace variables in the service file template
    sed -e "s|{DIRECTORY}|$_targetDirectory|g" -e "s|{EXE}|$_execName|g" -e "s|{USER}|$_userName|g" -e "s|{GROUP}|$_userName|g" $_serviceTemplateName > $_systemServiceFileLocation/$_serviceTemplateName

    #reload the deamon, in case is was already installed before    
    systemctl daemon-reload

    #enable the service so that it starts on boot
    systemctl enable $_serviceTemplateName

    #start the service
    systemctl start $_serviceTemplateName

    echo agent installation finished
}

#parse command line arguments

while [ "$1" != "" ]; do
    case $1 in
        -aui | --authentication-identity )  shift
            if [ "$1" = "SecurityModule" ] || [ "$1" = "Device" ]; then
                _identity=$1
            else
                echo "Possible values for authentication-identity are: SecurityModule or Device"
                usage
                exit 1
            fi	
                                    ;;
        -aum | --authentication-method )  shift
            if [ "$1" = "SymmetricKey" ] || [ "$1" = "SelfSignedCertificate" ]; then
                _authenticationMethod=$1
            else
                echo "Possible values for authentication-method are: SymmetricKey or SelfSignedCertificate"
                usage
                exit 1
            fi	
                                    ;;
        -f | --file-path )  shift
                                    _filePath=$1
                                    ;;
		-hn | --host-name )  shift
									_hostName=$1
									;;
		-di | --device-id)  shift
									_deviceId=$1
									;;
        -u | --uninstall )          shift
                                    _mode="uninstall"
                                    ;;
        -i | --install )            shift
                                    _mode="install"
                                    ;;
        -h | --help )               usage
                                    exit
                                    ;;
        * )                         usage
                                    exit 1
    esac
    shift
done

_scriptDir=$( cd "$(dirname "${BASH_SOURCE[0]}")" ; pwd -P )

_authenticationMethod=${_authenticationMethod/SymmetricKey/SasToken}

if [[ $_mode == "uninstall" ]]; then
    uninstallagent
elif [[ $_mode == "install" ]]; then
    installagent
else
    usage
fi
