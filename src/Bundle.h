/* 
 * File:   Bundle.h
 * Author: opus
 *
 * Created on 16 de Março de 2016, 09:51
 */

#ifndef BUNDLE_H
#define	BUNDLE_H

#define BUNDLEITEMS 73

const struct bundle{
    uint8_t pointer;
    uint8_t type;
    uint8_t clazz;
    uint8_t allow;
    const char * bkey;
    const char * prompt;
    const char * msg;
} bundle[BUNDLEITEMS] = {
    
          {100, 0, 0, 2,     "",                       "Carregar Metodo",                      "Config. do Instrumento" },
          {101, 0, 0, 0,     "",                       "Salvar Metodo",                        "" },
            {0, 1, 0, 2,     "prof_file",              "Mudar Metodo",                         "Entre nome do arq." },
          {102, 0, 0, 0,     "",                       "Ajustar data/hora",                    "" },
            {0, 1, 0, 2,     "language_file",          "Mudar Idioma",                         "Language file :" },
            {1, 2, 0, 0,     "",                       "Parametros de analise",                "Parametros de analise" },
            {6, 2, 0, 0,     "",                       "Controle de Press\x83 o",              "Controle de Press\x83 o" },
            {7, 2, 0, 0,     "",                       "Controle de Desvio",                   "Controle de Desvio" },
            {2, 2, 0, 0,     "",                       "Op\x81 \x85 es de Analise",            "Op\x81 \x85 es de Analise" },
            {5, 2, 0, 0,     "",                       "Controle de Qualidade",                "Controle de Qualidade" },
            {3, 2, 0, 0,     "",                       "Op\x81 \x85 es de Impress\x83 o",      "Op\x81 \x85 es de Impress\x83 o" },
            {4, 2, 0, 2,     "",                       "Calibra\x81 \x83 o do Sensor",         "Calibra\x81 \x83 o do Sensor" },           
            {0, 1, 0, 2,     "logonstart",             "Autenticar apos reset",                "Questionar usuario\nsobre senha\napos o reset ?" },
//13
            {0, 1, 1, 0,     "sample_weight",          "Massa Padrao",                         "Defina\nMassa" },
            
            {0, 1, 1, 0,     "sphere_vol_g",           "Volume da Esfera [G]",                 "Volume da\nEsfera [G]" },
            {0, 1, 1, 0,     "sphere_vol_m",           "Volume da Esfera [M]",                 "Volume da\nEsfera [M]" },
            {0, 1, 1, 0,     "sphere_vol_p",           "Volume da Esfera [P]",                 "Volume da\nEsfera [P]" },
         
 
            {0, 1, 1, 2,     "sphere_volume",          "Volume da Esfera",                     "Volume da\nEsfera" },
            {0, 1, 1, 0,     "analysis_rsd",           "Limite de Desvio",                     "Limite de\nDesvio" },
            {0, 1, 1, 0,     "runs_averaged",          "Analises para a Media",                "Analises\npara media" },
            {0, 1, 1, 0,     "max_runs",               "Maximo de Analises",                   "Maximo de\nanalisess" },
            {0, 1, 1, 0,     "flush_type",             "Tipo de Fluxo",                        "Tipo de\nFluxo" },
            {0, 1, 1, 0,     "flush_time",             "Tempo de Fluxo",                       "Tempo de\nFluxo" },
            {0, 1, 1, 0,     "flush_cicles",           "Ciclos de Fluxo",                      "Ciclos de\nFluxo"  },
            {0, 1, 1, 0,     "flush_cicle_time",       "Tempo de Ciclo",                       "Tempo de\nFluxo" },
            
            
            {0, 1, 1, 2,     "vcell_g",                "Volume da Celula [G]",                 "Volume da\nCelula [G]" },
            {0, 1, 1, 2,     "vcell_m",                "Volume da Celula [M]",                 "Volume da\nCelula [M]" },
            {0, 1, 1, 2,     "vcell_p",                "Volume da Celula [P]",                 "Volume da\nCelula [P]" },
            
            {0, 1, 1, 2,     "vadded_g",               "Volume Adicionado [G]",                "Volume da\nAdicionado [G]" },
            {0, 1, 1, 2,     "vadded_m",               "Volume Adicionado [M]",                "Volume da\nAdicionado [M]" },
            {0, 1, 1, 2,     "vadded_p",               "Volume Adicionado [P]",                "Volume da\nAdicionado [P]" },
            
            
//            {0, 1, 1, 2,     "vcell",                  "Volume da Celula (Vcell)",             "Volume da\nCelula" },
//            {0, 1, 1, 2,     "vadded",                 "Volume Adicionado",                    "Volume de\nExpansao" },
            
            
//10-23            
            {0, 1, 6, 0,     "vent-pressure",          "Limite para Ventilacao",               "Pressao de\nVentilacao" },
            {0, 1, 6, 0,     "pulse-pressure",         "Pressao Pulso de Fluxo",               "Pressao de \nPulso" },
            {0, 1, 6, 0,     "load-pressure",          "Pressao de Alvo",                      "Pressao de\nCarga" },            
            {0, 1, 6, 0,     "load-trigfactor",        "Fator de Disparo",                     "Fator de\nDisparo" },            
            {0, 1, 6, 0,     "use_rate_to_load",       "Usar Razao no Disparo",                "Limitar razao\nde subida\nna carga ?" }, 
            {0, 1, 6, 0,     "load-prate",             "Fator de Razao de Disparo",            "Razao de\nSubida" }, 
//6-29                       
            {0, 1, 7, 0,     "load_driftwstart",       "Janela de Desvio",                     "Janela de\ndesvio" },
            {0, 1, 7, 0,     "load_driftwend",         "Limite da Janela de Desvio",           "Limite de\ndesvio" },
            {0, 1, 7, 0,     "abs_driftlimit",         "Limite de Instabilidade",              "Limite para\ninstavel" },            
            {0, 1, 7, 0,     "rel_driftlimit",         "Usar limite de Desvio",                "Limite para\nDesvio" },            
            {0, 1, 7, 0,     "equilibration_time",     "Tempo para Equilibrio",                "Tempo para\nEquilibrio" }, 
            {0, 1, 7, 0,     "zero_driftwstart",       "Janela de Desvio no Zero",             "Janela para\nDesvio" },            
            {0, 1, 7, 0,     "zero_driftwend",         "Limite da Janela no Zero",             "Limite para\nZero" },        
            {0, 1, 7, 0,     "blockon_driftfail",      "Bloquear caso > desvio",               "Abortar se\namostra desviando ?" },        
//8-37
            {0, 1, 2, 0,     "usesequencer",           "Usar Sequenciador",                    "Usar sequenciador para\natualizar\nindentificacao ?" },
            {0, 1, 2, 0,     "info_gap",               "Tempo para Informaçao",                "Tempo entre\nexposicoes" },
            {0, 1, 2, 0,     "sidhistoryfile",         "Arquivo de Histrico",                  "Arquivo de historico :" },
            {0, 1, 2, 0,     "storehistory",           "Arm. Historico",                       "AQtualizar historico\napos dados\ndefinidos ?" },
            {0, 1, 2, 0,     "autostore",              "Auto armazenar Resultados",            "Armazenar resultados\nautomaticamente ?" },
            {0, 1, 2, 0,     "asktostore",             "Questionar armazenamento",             "Perguntar sobre\narmazenamento\nde resultados ?" },
            {0, 1, 2, 0,     "storefile",              "Arquivo de resultados",                "Arquivo de resultados:" },
            {0, 1, 2, 0,     "singlerun",              "Exposição simples",                    "Analisar com uma\nexposicao ?" },
            {0, 1, 2, 0,     "skip_first_run",         "Descartar primeira exposição",         "Descartar primeira\nexposicao?" },
            {0, 1, 2, 0,     "monitor_on_term",        "Monitorar no terminal",                "Registrar atividade\nno terminal ?" },
//10-47
            {0, 1, 3, 0,     "autoprint",              "Auto imprimir Resultados",             "Imprimir resultados\nautomaticamente ?" },
            {0, 1, 3, 0,     "asktoprint",             "Questionar impressao",                 "Perguntar sobre\nimpressao\nde resultados ?" },
            {0, 1, 3, 0,     "print_runs",             "Imprimir Individuais",                 "Adicionar tabela\ncom individuais\nno relatorio ?" },
            {0, 1, 3, 0,     "print_extra_data",       "Imprimir dados extras",                "Adicionar extras\nno relatorio ? " },
            {0, 1, 3, 0,     "print_qc",               "Imprimir CQ",                          "Add controle de\nqualidade\nno relatorio ?" },
            {0, 1, 3, 0,     "header_type",            "Tipo de Cabecalho",                    "Cabecalho do Relatorio :" },
            {0, 1, 3, 0,     "time_format",            "Formato da Data",                      "Formato da data :" },
            {0, 1, 3, 0,     "eject_print_report",     "Ejetar Relatorio",                     "Provisionar espaco\npara leitura na\nimpressao ?" },
//08-55
            {0, 1, 4, 0,     "zero_transducer",        "Zerar Transdutor",                     "Zerar transdutor\napos cada exposicao ?" },           
            {0, 1, 4, 0,     "tempa0",                 "Temp. Calib Curve A0",                 "Temp. curve\nY intercept" },
            {0, 1, 4, 0,     "tempa1",                 "Temp. Calib Curve A1",                 "Temp. curve\nSlope" },
            {0, 1, 4, 0,     "pressure_a0",            "Press Calib Curve A0",                 "Press curve\nY intercept" },
            {0, 1, 4, 0,     "pressure_a1",            "Press Calib Curve A1",                 "Press curve\nSlope" },
            {0, 1, 4, 0,     "pressure offset",        "Press Offset",                         "Press curve\nOffset" },
            
//06-61
            {0, 1, 5, 0,     "rsdlimit",               "Limite de Desvio",                     "Desvio maximo\nda analise" },
            {0, 1, 5, 0,     "alarm_bad_rsd",          "Informar Desvio Ruim",                 "Bloquear analise e\ninformar sobre\ndesvio ruim ?" },
            {0, 1, 5, 0,     "rangehigh",              "Limite superior",                      "Controle de qualidade\nvalor maximo" },
            {0, 1, 5, 0,     "rangelow",               "Limite inferior",                      "Controle de qualidade\nvalor minimo" },
//4-65 
            
};




#ifdef	__cplusplus
extern "C" {
#endif

#ifdef	__cplusplus
}
#endif

#endif	/* BUNDLE_H */

