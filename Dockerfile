FROM pierremarion23/latbuilder:dep

RUN cd $HOME && ./waf configure --prefix $HOME/latsoft --boost $HOME/dependencies --fftw $HOME/dependencies --ntl $HOME/dependencies && ./waf build && ./waf install
