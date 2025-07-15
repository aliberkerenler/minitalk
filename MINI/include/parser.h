#ifndef PARSER_H
#define PARSER_H

typdef struct s_redir{
    


} t_redir;

typdef struct s_pipe{
    


} t_pipe;

typdef struct s_command{
    


} t_command;

typdef struct s_line{
    
    t_command *command;
    t_pipe *pipe;
    

} t_line;












#endif