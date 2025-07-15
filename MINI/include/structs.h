// include/structs.h

#ifndef STRUCTS_H
# define STRUCTS_H

// Genel shell durumu için tek global değişken (eğer gerekiyorsa, konu kısıtlamalarına dikkat!) 
// extern volatile sig_atomic_t g_signal_status; // Örnek global sinyal değişkeni

// Yönlendirme türleri
typedef enum e_redir_type
{
    REDIR_IN,       // < [cite: 64]
    REDIR_OUT,      // > [cite: 65]
    REDIR_HEREDOC,  // << [cite: 66]
    REDIR_APPEND    // >> [cite: 68]
}   t_redir_type;

// Yönlendirme bilgisi
typedef struct s_redir
{
    t_redir_type    type;      // Yönlendirme tipi
    char            *file;     // Hedef dosya veya sınırlayıcı (heredoc için)
    struct s_redir  *next;     // Bir sonraki yönlendirme
}   t_redir;

// Komut yapısı
// Pipe'lar için, bir sonraki komutu işaret eden bir pointer veya
// bir komut listesi/dizisi tutan üst bir yapı olabilir.
typedef struct s_command
{
    char            **args;       // Komut ve argümanları (örneğin: {"ls", "-l", NULL})
    t_redir         *redirs;      // Komutla ilişkili yönlendirmelerin listesi
    int             is_builtin;   // Bu bir dahili komut mu? (Evet/Hayır)
    int             pid;          // Çalışan sürecin PID'si (yürütme aşamasında kullanılır)
    struct s_command *next_pipe; // Pipe varsa bir sonraki komuta işaret eder 
}   t_command;

// Shell'in genel durumu
typedef struct s_shell
{
    char            **envp;           // Ortam değişkenleri (getenv ile alınır, export/unset ile güncellenir) [cite: 46, 70]
    int             last_exit_status; // Son çalıştırılan komutun çıkış durumu ($?) [cite: 71]
    // Başka global durum değişkenleri (örneğin signal_status) buraya dahil edilmez
    // Konuya göre global değişken sadece sinyal numarası tutmalı, struct değil. [cite: 54, 55]
}   t_shell;

#endif