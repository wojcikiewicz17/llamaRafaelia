#include <stdio.h>
#include <math.h>

/*
 * Demo simbólico-técnico:
 * "campo efetivo" como composição de fatores (placeholders físicos)
 */

double campo_efetivo(double higgs, double casimir, double em, double sc) {
    // composição simples (exemplo didático)
    return (higgs * 0.25) + (casimir * 0.25) + (em * 0.25) + (sc * 0.25);
}

int main(void) {
    double higgs   = 0.92;  // fator Higgs (normalizado)
    double casimir = 1.08;  // energia de vácuo (normalizada)
    double em      = 0.97;  // eletromagnetismo
    double sc      = 1.12;  // supercondutividade

    double g_eff = campo_efetivo(higgs, casimir, em, sc);

    printf("Antigrav Demo :: g_eff = %.6f\n", g_eff);

    if (g_eff < 1.0)
        printf("Estado: redução gravitacional\n");
    else if (g_eff > 1.0)
        printf("Estado: amplificação gravitacional\n");
    else
        printf("Estado: neutro\n");

    return 0;
}
