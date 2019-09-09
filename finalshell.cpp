#include <bits/stdc++.h>
#include<unistd.h>
#include<sys/wait.h>
#include<sys/stat.h>
#include<fcntl.h>
#include <fstream>
#include <stdio.h>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

using namespace std;
map<string, string> aliasmap;
string user;
string hostname;
string ps;
bool normalpipe=false;
bool pipewithio=false;
bool normalIO=false;
bool complexIO=false;

int exitstatus;


char** spacetokenize(string str) //space tokenize
{
 // cout<<"string to space tokenizer: "<<str<<endl;
  stringstream query(str);
  string temp;
  int i=0;
  int j;
  char** args = new char*[10];
   while(getline(query,temp,' '))
   {
    //aliascount++;
    //cout<<"token in spacetokenizer: "<<temp<<endl;
    j=0;
    int length = temp.size (); //token ki size
    char *immediate = new char[length+1];

    for (; j < length; j++)
        {
         immediate[j] = temp[j]; // copying token
        }
        immediate[length]='\0';
        args[i] = immediate;// storing it in final array
        i++;
  }

  return args;
 }

char** tokenize(string str)// = tokenize
{
 // cout<<"string to space tokenizer: "<<str<<endl;
  stringstream query(str);
  string temp;
  int i=0;
  int j;
  char** args = new char*[10];
   while(getline(query,temp,'='))
   {

    j=0;
    int length = temp.size (); //token ki size
    char *immediate = new char[length+1];

    for (; j < length; j++)
        {
         immediate[j] = temp[j]; // copying token
        }
        immediate[length]='\0';
        args[i] = immediate;// storing it in final array
        i++;
  }

  return args;


 }

 void handlealias(string str)// handling alias
 {
 	//cout<<"inside handle alias"<<endl;
    string key="";
    for(int i=6;str[i]!='=';i++)
    {
      key=key+str[i];
    }
   // cout<<"key: "<<key<<endl;
    int i=0;
    while(str[i]!='=')
      i++;
    i++;
    while(str[i]==' ')
      i++;
    int count=0;
    int j=i;
    while(str[i]!='\0')
    {
      count++;
      i++;
    }
    char* stemp = new char[count+1];
    int k=0;
    while(count--)
    {
      stemp[k]=str[j];
      k++;
      j++;
    }
    string value=stemp;
    //cout<<"value: "<<value<<endl;

    auto itr=aliasmap.begin();
    if(aliasmap.find(key)!=aliasmap.end())// if key  found then just update value
    {
      itr=aliasmap.find(key);
      itr->second=value;

    }
    else if (aliasmap.find(value)!= aliasmap.end())// transitivity
    {
            itr=aliasmap.find(value);
            aliasmap.insert(pair<string, string>(key,itr->second));

    }
    else // completely new
    {
      aliasmap.insert(pair<string, string>(key, value));
    }
    
    //exit(0);         

 }

void evaluatePipeline(string str) //without IO
{
  //cout<<"in pipe"<<endl;
  stringstream query(str);
  string temp;
  int tokens=0;
  vector<string> vec; // pointer to each tuple

  while(getline(query,temp,'|'))
  { tokens++;
    vec.push_back(temp);

  }
  
    int RD = 0;
    int fd[2];
    for(int i=0; i<tokens;i++)
    {

      char** cmd = spacetokenize(vec[i]);
      pipe(fd);
      int pid=fork();

      if(pid==0)
      {
        if(i==0)
        {
          //cout<<"first"<<endl;
          dup2(fd[1],1);
          close(fd[0]);
          close(fd[1]);
          execvp(cmd[0],cmd);
        }
        else if (i==tokens-1)
        {
          //cout<<"last"<<endl;
          dup2(RD,0);
          close(fd[0]);
          close(fd[1]);
          //string cmd1=cmd[0];
          execvp(cmd[0],cmd);
        }
        else
        {
          //cout<<"inter"<<endl;
          dup2(RD,0);
          dup2(fd[1],1);
          close(fd[0]);
          close(fd[1]);
          //string cmd1=cmd[0];
          execvp(cmd[0],cmd);
        }
      }
      else
      {
        wait(NULL);
        close(fd[1]);
        RD = fd[0];
      }


    }


}

void evaluatePipelineIO(string str) //pipe with io
{
  //cout<<"in pipe"<<endl;
  stringstream query(str);
  string temp;
  int tokens=0;
  vector<string> vec; // pointer to each tuple

  while(getline(query,temp,'|'))
  { tokens++;
    vec.push_back(temp);

  }

    int RD = 0;
    int fd[2];
    for(int i=0; i<tokens;i++)
    {


      pipe(fd);
      int pid=fork();

      if(pid==0)
      {
        if(i==0)
        {
          char** cmd = spacetokenize(vec[i]);
         // cout<<"first"<<endl;
          dup2(fd[1],1);
          close(fd[0]);
          close(fd[1]);
          execvp(cmd[0],cmd);
        }
        else if (i==tokens-1)
        {
         // cout<<"last"<<endl;
          string str=vec[i];
          string cmd="";
          string file="";
          int i=0;
          int j=str.length();
          while(str[i]!='>')
          {
            cmd=cmd+str[i];
            i++;
          }
          while(str[j]!='>')
          {
            file=file+str[j];
            j--;
          }
          reverse(file.begin(),file.end());
          char** ptr=spacetokenize(cmd);
          int fd1= open(file.c_str(),O_CREAT|O_TRUNC|O_WRONLY);
          fchmod(fd1,S_IRWXU|S_IRWXG|S_IRWXO);
          dup2(RD,0);//reading from previous output
          dup2(fd1,1);//writing into file
          close(fd[0]);
          close(fd[1]);
          execvp(ptr[0],ptr);
        }
        else
        {
          char** cmd = spacetokenize(vec[i]);
          //cout<<"inter"<<endl;
          dup2(RD,0);
          dup2(fd[1],1);
          close(fd[0]);
          close(fd[1]);
          //string cmd1=cmd[0];
          execvp(cmd[0],cmd);
        }
      }
      else
      {
        wait(NULL);
        close(fd[1]);
        RD = fd[0];
      }


    }


}


void evaluate (string str) // alias added
{

	 // cout<<"inside evaluate"<<endl;
	 // cout<<str<<endl;
	  string initial = "/bin/";
      stringstream query (str);
      string temp;
      char ** args=new char*[20];
      int i = 0;
      char command[15];
      bool flag = false;
      string temp2;
      while (getline (query, temp, ' '))
        {
	          if(flag == true)
	          {
	          	temp2 = temp2 + temp;// getting path after command
	          }
	          int length = temp.size ();
	          char *immediate = new char[length+1];
	          for (int j = 0; j < length; j++)
	            {
	              immediate[j] = temp[j];
	            }
	          immediate[length]='\0';
	          args[i] = immediate;
	          i++;
	          flag = true;

        }// tokenize
       // cout<<"hi "<<args[0]<<endl;
        //cout<<"bye"<<args[1]<<endl;

        if (!strcmp(args[0],"cd")) //0 means successful
        {
          if(*args[1] == '~')
            chdir("/home/tushar");
          else
        	  //chdir(temp2.c_str());
           // cout<<"path:"<<args[1]<<endl;
            //const char * x = args[1];
            chdir(args[1]);
           // exit(0);
        }

      else
      {

        if(aliasmap.find(args[0])==aliasmap.end())
      	 execvp (args[0], args);//if alias not found
        else
        {
        	//cout<<"x"<<endl;
          auto itr = aliasmap.find(args[0]);
          char**ptr = spacetokenize(itr->second);
          //cout<<ptr[0]<<endl;
          //cout<<ptr[1]<<endl;
          execvp (ptr[0], ptr);
        }
    }
}

void evaluateIO(string str)
{

  stringstream query(str);
  string temp;
  vector<string> vec; // pointer to each tuple

  while(getline(query,temp,'>'))
  {
    vec.push_back(temp);
  }
  char** ptr=spacetokenize(vec[0]);
  int fd= open(vec[1].c_str(),O_CREAT|O_TRUNC|O_WRONLY);
  fchmod(fd,S_IRWXU|S_IRWXG|S_IRWXO);
  dup2(fd,1);
  execvp(ptr[0],ptr);


}
void evaluateIO2(string str)
{
	string cmd="";
    string file="";
    int i=0;
    while(str[i] != '>')
    {
      cmd=cmd+str[i];
      i++;
    }
    int j=str.length()-1;
    while(str[j]!='>')
    {
      file=file+str[j];
      j--;
    }

    reverse(file.begin(),file.end());
    //cout<<"command "<<cmd<<" "<<"filename "<<file<<endl;
    char** ptr=spacetokenize(cmd);
    //int fd= open(file.c_str(),O_APPEND|O_WRONLY);
    int fd= open(file.c_str(),O_APPEND|O_WRONLY);
    fchmod(fd,S_IRWXU|S_IRWXG|S_IRWXO);
   // cout<<fd<<endl;
    dup2(fd,1);
   // cout<<ptr[0]<<endl;
    //cout<<ptr[1]<<endl;

    execvp(ptr[0],ptr);

}

void evaluateSpecial1(string str)
{
  cout<<getpid()<<endl;
}

void evaluateSpecial2(string str)
{
  cout<<exitstatus<<endl;
}

void bashHistory(string str)
{
  ofstream ostream;
  ostream.open("history.txt",std::ofstream::out | std::ofstream::app);
  ostream << str << endl;
  ostream.close();

}

void evaluatemultimedia(string str)
{
	//cout<<"inside multimedia"<<endl;
	string filelocation="/home/tushar/codes/OS1/";
	char** ptr = spacetokenize(str);
	//cout<<"token 1"<<ptr[0]<<endl;
	//cout<<"token 2"<<ptr[1]<<endl;
	//cout<<"hi"<<endl;
	string filename=ptr[1];
	//cout<<"filename:"<<filename<<endl;
	string extension="";
	int j=filename.length()-1;
	while(filename[j]!='.')
	{
		extension=extension+filename[j];
		j--;
	}
	reverse(extension.begin(),extension.end());
	//cout<<"extension:"<<extension<<endl;
	string path;

	string data;
	ifstream infile;
    infile.open("/home/tushar/codes/OS1/multimedia.txt");
    while(getline(infile,data))
    {
    	stringstream query(data);
  		string temp;
  		vector<string> vec;

  		while(getline(query,temp,'='))
  			{
  			  vec.push_back(temp);
 			}
 		//cout<<"vec[0]"<<vec[0]<<endl;
 		if(vec[0]==extension)
 		{
			path=vec[1];
			break;
		}
	}
infile.close();
//cout<<"path:"<<path<<endl;
//cout<<"filename:"<<filename<<endl;

char** ptr1=new char*[3];

int length=path.length();
char*immediate=new char[length+1];
for(int i=0;i<length;i++)
{
	immediate[i]=path[i];
}
immediate[length]='\0';
ptr1[0]=immediate; // path of bin

string location = filelocation+filename;
//cout<<"location:"<<location<<endl;

int length1=location.length();
char*immediate1=new char[length1+1];
for(int i=0;i<length1;i++)
{
	immediate1[i]=location[i];
}
immediate1[length1]='\0';
ptr1[1]=immediate1;
//cout<<"ptr[0]:"<<ptr1[0]<<endl;
//cout<<"ptr[1]:"<<ptr1[1]<<endl;
ptr[3]=NULL;
execlp(ptr1[0],ptr1[0],ptr1[1],NULL);



}
void initshell()
{
	string data;
	ifstream infile;
  infile.open("/home/tushar/codes/OS1/bash.txt");
  int count=0;
  int i,j;
    while(getline(infile,data))
    { i=0;
      j=0;
      count++;
      //cout<<"count:"<<count<<endl;
      string var,path;
    	for(i;data[i]!='=';i++)
      {
        var=var+data[i];
      }
      for(int j=data.length();data[j]!='=';j--)
      {
        path=path+data[j];
      }
      reverse(path.begin(),path.end());
      //cout<<"var: "<<var<<endl;
      //cout<<"path: "<<path<<endl;
      /*stringstream query(data);
  		string temp;
  		vector<string> vec;

  		while(getline(query,temp,'='))
  		{
  			  vec.push_back(temp);
 		}
 		auto it = vec.begin();
 		while(it!=vec.end())
 		{
 			cout<<*it<<endl;
 			it++;
 		}*/

 		if(var=="HOSTNAME")
 			hostname=path;

 		if(var=="USER")
 			user=path;

 		if(var=="PS1")
 			ps=path;

 	}
infile.close();
}

bool checkalias(string str)
{
    //cout<<"in check alias"<<endl;
	char**ptr= spacetokenize(str);
	if(strcmp(ptr[0],"alias")==0)
		return true;
	else
		return false;
}

bool checkmultimedia(string str)
{
    //cout<<"in check multimedia"<<endl;
	char**ptr= spacetokenize(str);
	//cout<<ptr[0]<<endl;
	if(strcmp(ptr[0],"open")==0)
		return true;
	else
		return false;
}
bool checkspecial1(string str)
{
    //cout<<"in check special 1"<<endl;
	if(str=="$$")
		return true;
	else
		return false;
}
bool checkspecial2(string str)
{
    //cout<<"in special 2"<<endl;
	if(str=="$?")
		return true;
	else
		return false;
}

void checkpipeline(string str) //***********
{
    //cout<<"in check pipeline"<<endl;
	bool x=false;
	bool y=false;
	int i=0;
	while(str[i]!='\0')
	{
		if(str[i]=='|')
			x=true;
		if(str[i]=='>')
			y=true;
			i++;
	}
	if(x==true && y==false)
		normalpipe=true;
	else if(x==true && y==true)
		pipewithio=true;
	return;
}
void checkIO(string str)//********************8
{
    //cout<<"in check IO"<<endl;
	int i=0;
	int count=0;
	while(str[i]!='\0')
	{
		if(str[i]=='>')
			count++;
			i++;
	}
	if(count==1)
		normalIO=true;
	else if(count==2)
		complexIO=true;
	return;

}

int main (int argc, char const *argv[])
{
  int flag=0;
 	//initshell();
 // cout<<"user:"<<user<<endl;
 // cout<<"hostname:"<<hostname<<endl;
  //cout<<"PS1:"<<ps<<endl;
//  string strinit=user+"@"+hostname+":"+ps;
  int status;
  while (1)
    {

    //  cout<<strinit;
      cout<<"--> ";
      string query;
      getline (cin, query);
      bashHistory(query);
      bool alias =false;
      bool multi=false;
      bool special1=false;
      bool special2=false;
      bool normal=false;
      normalpipe=false;
	  pipewithio=false;
	  normalIO=false;
	  complexIO=false;

      alias=checkalias(query);
      multi=checkmultimedia(query);
      special1=checkspecial1(query);
      special2=checkspecial2(query);
      checkpipeline(query);
      checkIO(query);
      if(alias==false && multi==false && special1==false && special2==false && normalpipe==false && pipewithio==false && normalIO==false && complexIO==false)
      	normal=true;

      int pid= fork();
      if(pid==0)
	{
      if(alias)
      	handlealias(query);
      else if(multi)
      	evaluatemultimedia(query);
      else if(special1)
      	evaluateSpecial1(query);
      else if(special2)
      	evaluateSpecial2(query);
      else if(normalpipe)
      	evaluatePipeline(query);
      else if(pipewithio)
      	evaluatePipelineIO(query);
      else if(normalIO)
      	evaluateIO(query);
      else if(complexIO)
      	evaluateIO2(query);
      else if(normal)
      	evaluate(query);

  	}
  	 waitpid(pid,&status,0);
  	 if ( WIFEXITED(status) )
    {
        exitstatus = WEXITSTATUS(status);

    }


    }
  return 0;
}
