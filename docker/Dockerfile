FROM ubuntu
RUN pwd
RUN apt-get -y update
RUN apt-get -y install sl htop psmisc psutils time iotop iputils-ping net-tools build-essential ssh sshfs bash-completion rsync
RUN apt-get -y install git screen stress tmux iperf libxml2-dev
 
RUN apt-get -y install openjdk-8-jdk ipython pypy python-dev python-pip virtualenvwrapper
RUN pip install requests flask virtualenvwrapper
RUN apt-get -y upgrade
RUN useradd -s /bin/bash -m drchecker
RUN usermod -aG sudo drchecker
ADD install.sh /tmp/install.sh
RUN CHMOD a+rx /tmp/install.sh
RUN su - drchecker -c /tmp/install.sh
CMD su - drchecker
