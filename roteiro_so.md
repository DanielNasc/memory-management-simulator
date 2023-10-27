# Introdução

- Olá, eu sou o galinho m(v)x, vou ser o seu guia nessa simulação

- Nesta simulação do nosso Mi(V)OS descobriremos como ocorre o gerenciamento de memória em um sistema operacional.

- Na barra superior está o espaço utilizado na sua memória no momento
> Destacar barra superior
- No centro você verá o código do processo que iremos executar no nosso sistema
> Destacar barra do centro
- Logo abaixo você pode ver o que acontece com a memória conforme o processo é executado
> Destacar o inferior

- Pelo propósito dessa simulação, iremos focar apenas na memória do espaço do usuário. Então vou escondi pra você a parte referente ao interior do sistema. <vergonha> Favor, respeitar ok? <blushed>

- Para controlar a execução pressione o botão [ESPAÇO] e iremos mostrar o que acontece. Tente!


## Continuação após código "hello world": Loop (Recursão) e Stack Oveflow

- Muito bem, você deve ter entendido. Certo?

- Agora iremos mostrar o que acontecesse quando temos um loop no programa.

<loop inicia>
<loop não para>

- Oh no! Que problemão, a nossa memória vai acabarrrrrrrrrrrrrrrrrrrrrrrrrrrrrr... (continua até pressionar espaço)

- Droga, eu travei, isso que acabou de acontecer é chamado de "Stack Overflow", não o site bobão!

## Swap

- Bem, podemos resolver o problema anterior usando uma estratégia chamada "swapping", eu vou mostrar pra você.

<mostrar swap>

- Esta nova partição aqui é chamada de... isso SWAP, é basicamente um arquivo no disco do PC. Vamos ver agora!

<loop continua e swap é preenchido, e para antes de travar>

- Ótimo!

## Multiprogramação (partição fixa)

- Mas eu posso fazer mais do que isso. Agora vamos ver o que acontecesse quando há vários programas sendo executados.

<carrega dois códigos main na tela>

- Queremos tirar proveito dos recursos da máquina, então faremos esses dois programas serem carregados simultaneamente.

- Poderemos então executar um após o outro

<mostrar execução single thread>

- Ou alternar entre os dois, em sequência. Isso é útil principalmente quando a múltiplas threads.

<mostrar execução multithread>

- É isso, eu tenho dois corações! h3h3

- Você deve ter notado os endereços que os programas estão usando colidem. Na verdade, nós não estamos usando os endereços da memória física. Isso facilita muitos processos pra nós. 
- Esse endereço (virtual) pode ser atribuído em tempo de compilação, tempo de carga, ou em tempo de execução, chamamos ele de memória lógica.
- Eu tenho um circuito especial pra lidar com isso no meu cérebro. O MMU! Ele faz a tradução dos endereços, e também que fará a proteção deles entre processos. Note que o endereçamento físico é realocado sempre que manipulamos a localização dos processos na memória, para um swap por exemplo.

## Multiprogramação (partição variável)

<Stack Overflow em um dos processos>

- Olhe, temos um problema, o segundo programa não pode continuar por quê não há espaço o suficiente na máquina.

- Podemos resolver isso, balanceando o uso da memória.

<mostrar alocação de tamanhos diferentes>


## Ponteiros e memória heap

- Ok! Agora vou te mostrar algo legal... Algo PERIGOSO!!! RADICAL! **Ponteiros**

<carregar demonstração da memória heap>

- Esse espaço no final é chamado de heap (ou monte), é aqui que fica a memória adicional. Muitos programas utilizam a heap quando querem crescer variavelmente, é um pouco de trabalho para mim, mas é um trabalho honesto.

## Segmentation Fault

- Espere, você não deveria acessar esta memória, o outro programa vai ter problema. Erro!!! Erro!!

- Como eu esperava, esse é mais um caso de Segmentation fault. Um programador responsável nunca permitiria ponteiros soltos assim. Eu não permitirei acessar além da fronteira. Segmentation fault! Ouviu bem?

## Nullpointer excpetion

<carregar código de acesso null>

- Espere, esse endereço não existe! O endereço 00 é especial, nós chamamos ele de NULL. É uma exceção especial pra facilitar o trabalho de alguns programadores. Nós chamamos isso de Null pointer Exception

## Multiprogramação com partição variável: first fit, best fit, worst fit

- Bem, você já entendeu como processos funcionam. Vamos ignorar esta parte por enquanto, e focar em entender o que acontece quando há **muitos** processos na memória!

<simulação com vários processos, alguns encerrando>

- Precisamos encaixar este novo processo, mas não há espaço o suficiente no momento, nós vamos então esperar pelos outros encerrarem para inserí-lo. Ou, eu posso mover alguns deles de menor prioridade para a swap, hihi

<simulação com vários processos, mais deles encerrando>

- Agora que temos espaço o suficiente podemos colocá-lo... Mas onde vamos por ele?

<simular first fit (alocando no começo)>

- Faz sentido, no primeiro espaço. Mas, Bem, a gente pode se prepar melhor para este problema do espaço no futuro. Que tal inserirmos em outro local?

<simulação best fit>

- Eu decidi colocar ele no melhor lugar, onde o processo vai deixar lacunas de espaço menores. Mas a gente pode fazer diferente!

<simulação worst fit>

- Dessa vez eu decidi colocar ele no pior lugar? Por quê, você me pergunta? Olha, os próximos processos vão ter tooodo esse espaço livre pra caber ali, isso é bom, não é mesmo?

## Paginação

- Bem, eu não estou satisfeito. Ainda há espaço que não estamos usando... E que tal se dividirmos o processo em pedaços?

<simulação de quebra do processo>

- Quanto aos endereços (virtuais) internos do programa, não se preocupe! Nós podemos usar uma tabela pra registrar a quebra, e deixamos a conversão ser feita por mim em uma TLB (Translation look-aside buffer) e meu poderoso MMU! h3h3

<apresentação da tabela de endereços>

- Pronto! Agora sim!

<fim da apresentação>
