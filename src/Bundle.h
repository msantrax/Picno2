/* 
 * File:   Bundle.h
 * Author: opus
 *
 * Created on 16 de Março de 2016, 09:51
 */

#ifndef BUNDLE_H
#define	BUNDLE_H

#define BUNDLEITEMS 66

const struct bundle{
    uint8_t pointer;
    uint8_t type;
    uint8_t clazz;
    const char * bkey;
    const char * prompt;
    const char * msg;
} bundle[BUNDLEITEMS] = {
    
          {100, 0, 0,     "",                       "Carregar Metodo",                      "Config. do Instrumento" },
          {101, 0, 0,     "",                       "Salvar Metodo",                        "" },
            {0, 1, 0,     "prof_file",              "Mudar Metodo",                         "Entre nome do arq." },
          {102, 0, 0,     "",                       "Ajustar data/hora",                    "" },
            {0, 1, 0,     "language_file",          "Mudar Idioma",                         "Language file :" },
            {1, 2, 0,     "",                       "Parametros de analise",                "Parametros de analise" },
            {6, 2, 0,     "",                       "Controle de Press\x83 o",              "Controle de Press\x83 o" },
            {7, 2, 0,     "",                       "Controle de Desvio",                   "Controle de Desvio" },
            {2, 2, 0,     "",                       "Op\x81 \x85 es de Analise",            "Op\x81 \x85 es de Analise" },
            {5, 2, 0,     "",                       "Controle de Qualidade",                "Controle de Qualidade" },
            {3, 2, 0,     "",                       "Op\x81 \x85 es de Impress\x83 o",      "Op\x81 \x85 es de Impress\x83 o" },
            {4, 2, 0,     "",                       "Calibra\x81 \x83 o do Sensor",         "Calibra\x81 \x83 o do Sensor" },           
            {0, 1, 0,     "logonstart",             "Autenticar apos reset",                "Questionar usuario\nsobre senha\napos o reset ?" },
//13
            {0, 1, 1,     "sample_weight",          "Massa Padrao",                         "Defina\nMassa" },
            {0, 1, 1,     "vcell",                  "Volume da Celula (Vcell)",             "Volume da\nCelula" },
            {0, 1, 1,     "vadded",                 "Volume Adicionado",                    "Volume de\nExpansao" },
            {0, 1, 1,     "sphere_volume",          "Volume da Esfera",                     "Volume da\nEsfera" },
            {0, 1, 1,     "analysis_rsd",           "Limite de Desvio",                     "Limite de\nDesvio" },
            {0, 1, 1,     "runs_averaged",          "Analises para a Media",                "Analises\npara media" },
            {0, 1, 1,     "max_runs",               "Maximo de Analises",                   "Maximo de\nanalisess" },
            {0, 1, 1,     "flush_type",             "Tipo de Fluxo",                        "Tipo de\nFluxo" },
            {0, 1, 1,     "flush_time",             "Tempo de Fluxo",                       "Tempo de\nFluxo" },
            {0, 1, 1,     "flush_cicles",           "Ciclos de Fluxo",                      "Ciclos de\nFluxo"  },
            {0, 1, 1,     "flush_cicle_time",       "Tempo de Ciclo",                       "Tempo de\nFluxo" },
//10            
            {0, 1, 6,     "vent-pressure",          "Limite para Ventilacao",               "Pressao de\nVentilacao" },
            {0, 1, 6,     "pulse-pressure",         "Pressao Pulso de Fluxo",               "Pressao de \nPulso" },
            {0, 1, 6,     "load-pressure",          "Pressao de Alvo",                      "Pressao de\nCarga" },            
            {0, 1, 6,     "load-trigfactor",        "Fator de Disparo",                     "Fator de\nDisparo" },            
            {0, 1, 6,     "use_rate_to_load",       "Usar Razao no Disparo",                "Limitar razao\nde subida\nna carga ?" }, 
            {0, 1, 6,     "load-prate",             "Fator de Razao de Disparo",            "Razao de\nSubida" }, 
//6                       
            {0, 1, 7,     "load_driftwstart",       "Janela de Desvio",                     "Janela de\ndesvio" },
            {0, 1, 7,     "load_driftwend",         "Limite da Janela de Desvio",           "Limite de\ndesvio" },
            {0, 1, 7,     "abs_driftlimit",         "Limite de Instabilidade",              "Limite para\ninstavel" },            
            {0, 1, 7,     "rel_driftlimit",         "Usar limite de Desvio",                "Limite para\nDesvio" },            
            {0, 1, 7,     "equilibration_time",     "Tempo para Equilibrio",                "Tempo para\nEquilibrio" }, 
            {0, 1, 7,     "zero_driftwstart",       "Janela de Desvio no Zero",             "Janela para\nDesvio" },            
            {0, 1, 7,     "zero_driftwend",         "Limite da Janela no Zero",             "Limite para\nZero" },        
            {0, 1, 7,     "blockon_driftfail",      "Bloquear caso > desvio",               "Abortar se\namostra desviando ?" },        
//8
            {0, 1, 2,     "usesequencer",           "Usar Sequenciador",                    "Usar sequenciador para\natualizar\nindentificacao ?" },
            {0, 1, 2,     "info_gap",               "Tempo para Informaçao",                "Tempo entre\nexposicoes" },
            {0, 1, 2,     "sidhistoryfile",         "Arquivo de Histrico",                  "Arquivo de historico :" },
            {0, 1, 2,     "storehistory",           "Arm. Historico",                       "AQtualizar historico\napos dados\ndefinidos ?" },
            {0, 1, 2,     "autostore",              "Auto armazenar Resultados",            "Armazenar resultados\nautomaticamente ?" },
            {0, 1, 2,     "asktostore",             "Questionar armazenamento",             "Perguntar sobre\narmazenamento\nde resultados ?" },
            {0, 1, 2,     "storefile",              "Arquivo de resultados",                "Arquivo de resultados:" },
            {0, 1, 2,     "singlerun",              "Exposição simples",                    "Analisar com uma\nexposicao ?" },
            {0, 1, 2,     "skip_first_run",         "Descartar primeira exposição",         "Descartar primeira\nexposicao?" },
            {0, 1, 2,     "monitor_on_term",        "Monitorar no terminal",                "Registrar atividade\nno terminal ?" },
//10
            {0, 1, 3,     "autoprint",              "Auto imprimir Resultados",             "Imprimir resultados\nautomaticamente ?" },
            {0, 1, 3,     "asktoprint",             "Questionar impressao",                 "Perguntar sobre\nimpressao\nde resultados ?" },
            {0, 1, 3,     "print_runs",             "Imprimir Individuais",                 "Adicionar tabela\ncom individuais\nno relatorio ?" },
            {0, 1, 3,     "print_extra_data",       "Imprimir dados extras",                "Adicionar extras\nno relatorio ? " },
            {0, 1, 3,     "print_qc",               "Imprimir CQ",                          "Add controle de\nqualidade\nno relatorio ?" },
            {0, 1, 3,     "header_type",            "Tipo de Cabecalho",                    "Cabecalho do Relatorio :" },
            {0, 1, 3,     "time_format",            "Formato da Data",                      "Formato da data :" },
            {0, 1, 3,     "eject_print_report",     "Ejetar Relatorio",                     "Provisionar espaco\npara leitura na\nimpressao ?" },
//08
            {0, 1, 4,     "zero_transducer",        "Zerar Transdutor",                     "Zerar transdutor\napos cada exposicao ?" },           
            {0, 1, 4,     "tempa0",                 "Temp. Calib Curve A0",                 "Temp. curve\nY intercept" },
            {0, 1, 4,     "tempa1",                 "Temp. Calib Curve A1",                 "Temp. curve\nSlope" },
            {0, 1, 4,     "pressure_a0",            "Press Calib Curve A0",                 "Press curve\nY intercept" },
            {0, 1, 4,     "pressure_a1",            "Press Calib Curve A1",                 "Press curve\nSlope" },
            {0, 1, 4,     "pressure offset",        "Press Offset",                         "Press curve\nOffset" },
            
//06
            {0, 1, 5,     "rsdlimit",               "Limite de Desvio",                     "Desvio maximo\nda analise" },
            {0, 1, 5,     "alarm_bad_rsd",          "Informar Desvio Ruim",                 "Bloquear analise e\ninformar sobre\ndesvio ruim ?" },
            {0, 1, 5,     "rangehigh",              "Limite superior",                      "Controle de qualidade\nvalor maximo" },
            {0, 1, 5,     "rangelow",               "Limite inferior",                      "Controle de qualidade\nvalor minimo" },
//4 
            
};




#ifdef	__cplusplus
extern "C" {
#endif

#ifdef	__cplusplus
}
#endif

#endif	/* BUNDLE_H */

