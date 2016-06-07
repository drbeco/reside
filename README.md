# reside

## Termina e fica residente

Muitos anos atrás eu tinha um livrinho delicioso de ler que não lembro exatamente o nome. Tinha uma capa azul, não chegava a 150 páginas, e tinha exemplos de códigos muito complexos para a época.

Eu acredito, se não me falha a memória, que o título era algo como **"Reside: termina e fica residente"**.

Os programas eram para DOS, e permitiam simular vários tipos de paralelismo, criar _daemons_, e até vírus!

Um belo dia emprestei a muito contra-gosto esse livro para algum amigo que também não lembro o nome e nunca mais vi o(s) dito(s) cujo (ou seja, perdi o livro e o amigo!).

## Data

Sempre fui muito organizado com meus backups, e notei que tenho até hoje comigo os códigos fielmente digitados do livro! Os arquivos executáveis estão com data de 13/Ago/2012 e os arquivos fontes com data de 28/Fev/2005, mas estas data estão incorretas. Em 2005 eu tive um problema com o computador da época e tive que usar o backup, e fui inexperiente ao recuperar meus dados, perdendo muitas datas originais de arquivos antigos. Os arquivos foram fontes foram recuperados, mas esqueci de preservar as datas. E os binários foram gerados várias vezes novamente, portanto as datas não são precisas.

Novamente, pela minha falha memória (e apoio de um ótimo histórico com data de compras dos meus antigos computadores), eu diria que estes arquivos são de 1993.

Em 1995 a Borland liberou o download gratuito do seu excelente compilador Turbo C versão 1.5, que é o compilador utilizado por esta biblioteca.

## Preservação histórica

Visando preservar estes arquivos para a posteridade e colaborar com a história da computação, estou criando este repositório.

Já tentei em vários momentos descobrir de onde são os originais, quem é (são) o(s) autor(es) do livro para dar os créditos, e já liguei para todos meus amigos perguntando se não tinha ficado com nenhum livro meu de nome esquecido, capa azulzinha (será mesmo azul?), mas não tive sucesso.

Posterguei muito em publicar estes arquivos para não violar direitos autorais, apesar de serem exercícios de um livro que incentivava a programação. Mas na data de hoje, 07/Jul/2016, e considerando a extemporaneidade da evolução dos computadores e sistemas operacionais (nenhum destes programas funcionará em sistemas modernos, pelo modo como acessam as interrupções de sistema), e do caráter didático acima de tudo, a publicação só trará benefícios para todos.

Se alguém sabe que livro é esse, terei o maior prazer de colocar os devidos créditos. Ficarei muito feliz em poder divulgar o nome do livro aqui, e se existir uma cópia ainda viva, quero um exemplar para minha biblioteca pessoal.

## Índice de arquivos:

### click

Emite um sinal sonoro quando no pressionamento  de  uma tecla, exemplificando a utilizacao da INT 9h (Teclado). 

* click.c
* click.exe

### multitar

Biblioteca de MultiTarefa para TURBOC versao 1.5 ou superior.

* multitar.h
* multitar.c

### printer

Filtra os codigos especiais do PC para impressora, exemplificando os programas residente ativados por interrupcao de Software, que neste caso e' a 17h.

* printer.c
* printer.exe

### rcopy

Copiador Residente de Arquivos. Exemplo de utilizacao da Biblioteca MULTITAR.C

* rcopy.c
* rcopy.prj
* rcopy.exe

### rdir

Diretorio Residente - Exemplo de utilizacao da Biblioteca RESIDE.C

* rdir.c
* rdir.prj
* rdir.exe
* 
### relogio

Liga um relogio no canto superior direito da tela, exemplificando a utilizacao da interrupcao de relogio.

* relogio.c
* relogio.exe
* 
### reside

Biblioteca de Residencia para Turbo C versao 1.5 ou superior

* reside.c
* reside.h

### scan

Desvia a interrupcao de teclado, e mostra os codigos (SCAN CODE) de cada tecla pressiona, bem como as teclas de SHIFT. O programa termina com  o  pressionamento da tecla ESC.

* scan.c
* scan.exe

### video

Liga ou Desliga o video atraves do pressionamento das teclas ALT ESC, exemplificando a manipulacao do video e da interrupcao de teclado (int 9h)

* video.c
* video.exe
