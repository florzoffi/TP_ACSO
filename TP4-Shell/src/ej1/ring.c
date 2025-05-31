#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main( int argc, char **argv ) {
    if ( argc != 4 ) {
        fprintf( stderr, "Uso: %s <n> <c> <s>\n", argv[0] );
        exit( EXIT_FAILURE );
    }
    int n = atoi( argv[1] );      
    int valor_inicial = atoi( argv[2] ); 
    int start = atoi( argv[3] );    
    if ( start >= n ) { exit(1); }
    if ( start < 0 ) { exit(1); }
    int pipes[n][2];
    for ( int i = 0; i < n; i++ ) {
        if ( pipe( pipes[i] ) == -1 ) {
            perror( "pipe" );
            exit( 1 );
        }
    }
    int pipe_final[2];
    if ( pipe( pipe_final ) == -1 ) {
        perror( "pipe final" );
        exit(1);
    }
    for ( int i = 0; i < n; i++ ) {
        pid_t pid = fork();
        if ( pid == -1 ) {
            perror( "fork" );
            exit( 1 );
        }

        if ( pid == 0 ) {
            for ( int j = 0; j < n; j++ ) {
                if ( j != i ) close( pipes[j][0] );
                if ( j != ( i + 1 ) % n ) close( pipes[j][1] );
            }
            close( pipe_final[0] );
            int valor;
            read( pipes[i][0], &valor, sizeof( int ) );
            valor++;
            if (i == ( start + n - 1 ) % n ) { write( pipe_final[1], &valor, sizeof( int ) ); } 
            else { write( pipes[(i + 1) % n][1], &valor, sizeof( int ) ); }
            exit( 0 );
        }
    }
    for ( int i = 0; i < n; i++ ) {
        if ( i != start ) close( pipes[i][1] ); 
        close( pipes[i][0] );
    }
    close( pipe_final[1] );
    write( pipes[start][1], &valor_inicial, sizeof( int ) );
    close( pipes[start][1] );
    int resultado_final;
    read( pipe_final[0], &resultado_final, sizeof( int ) );
    close( pipe_final[0] );
    printf( "El resultado es: %d\n", resultado_final );
    
    for ( int i = 0; i < n; i++ ) { wait( NULL ); }
    return 0;
}