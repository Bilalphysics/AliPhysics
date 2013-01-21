/************************************************************************* 
 * Copyright(c) 1998-2008, ALICE Experiment at CERN, All rights reserved. * 
 *                                                                        * 
 * Author: X. Sanchez Castro                                              * 
 * Contributors are mentioned in the code where appropriate.              * 
 *                                                                        * 
 * Permission to use, copy, modify and distribute this software and its   * 
 * documentation strictly for non-commercial purposes is hereby granted   * 
 * without fee, provided that the above copyright notice appears in all   * 
 * copies and that both the copyright notice and this permission notice   * 
 * appear in the supporting documentation. The authors make no claims     * 
 * about the suitability of this software for any purpose. It is          * 
 * provided "as is" without express or implied warranty.                  * 
 **************************************************************************/

#include <TCanvas.h>
#include <TTree.h>
#include <TFile.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TH3F.h>
#include <TPDGCode.h>
#include <TDatabasePDG.h>
#include <TClonesArray.h>
#include <TROOT.h>

#include "AliOADBContainer.h"

#include "AliAODMCHeader.h"
#include "AliAODMCParticle.h"
#include "AliGenHijingEventHeader.h"

#include "AliAODEvent.h"
#include "AliAODv0.h"
#include "AliAODcascade.h"

#include "AliCFContainer.h"
#include "AliCentrality.h"

#include "AliPID.h"
#include "AliPIDResponse.h"
#include "AliAODPid.h"

#include "AliInputEventHandler.h"
#include "AliAnalysisManager.h"

#include "AliAnalysisTaskLambdaOverK0sJets.h"

extern TROOT *gROOT;


ClassImp(AliAnalysisTaskLambdaOverK0sJets)

// Global variables:
static Int_t    nbins = 100;                 // Number of bins for l, pt, mass for V0
static Int_t    nbinsPhi = 120;              // Number of bins for Phi
static Int_t    nbinsdPhi = 42;              // Number of bins for dPhi
static Int_t    nbinsdEta = 40;              // Number of bins for dEta
static Int_t    nbinPtLP = 200;
static Int_t    nbinsVtx = 20;

static Float_t pMin = 0.0;                  // Lower cut for transverse momentum
static Float_t pMax = 10.;                  // Max cut for transverse momentum for V0
static Float_t ptMaxLP = 50.;               // Max cut for transverse momentum LP

static Float_t lMin = 0.0;                  // Limits in the histo for fidutial volume
static Float_t lMax = 100.;                 // Limits in the fidutial volume

//
//  
//

AliAnalysisTaskLambdaOverK0sJets::AliAnalysisTaskLambdaOverK0sJets(const char *name) :
  AliAnalysisTaskSE(name),

  fAOD(0), fIsMC(kFALSE), fUsePID(kFALSE), fCentMin(0.), fCentMax(90.), fDoQA(kFALSE), fTrigPtMin(8.), fTrigPtMax(20.), fTrigEtaMax(0.8), fCheckIDTrig(kFALSE), fSeparateInjPart(kTRUE), fEndOfHijingEvent(-1),  fPIDResponse(0),

  fMinPtDaughter(0.160), fMaxEtaDaughter(0.8), fMaxDCADaughter(1.0), fYMax(0.5), fDCAToPrimVtx(0.1), fMinCPA(0.998), fNSigma(3.0), fMinCtau(0.), fMaxCtau(3.), fIdTrigger(-1), fIsTrigFromV0daug(0), fIsV0LP(0), fPtV0LP(0.), fIsSndCheck(0),

  fOutput(0), fOutputQA(0), fEvents(0), fCentrality(0), fPrimaryVertexX(0), fPrimaryVertexY(0), fPrimaryVertexZ(0), fCentMult(0), fdEdx(0), fdEdxPid(0), 

  fTriggerMCPtCent(0), fTriggerPtCent(0), fTriggerEtaPhi(0), fCheckTriggerFromV0Daug(0), fTriggerComingFromDaug(0), fTriggerIsV0(0), fCheckIDTrigPtK0s(0), fCheckIDTrigPhiK0s(0), fCheckIDTrigPtLambda(0), fCheckIDTrigPhiLambda(0),  fInjectedParticles(0),

  fK0sMCPt(0), fK0sMCPtRap(0), fK0sMCPtPhiEta(0), fK0sAssocPt(0), fK0sAssocPtArm(0), fK0sAssocPtRap(0), fK0sAssocPtPhiEta(0), fK0sMCResPhi(0), fLambdaMCPt(0), fLambdaMCPtRap(0), fLambdaMCPtPhiEta(0), fLambdaAssocPt(0), fLambdaAssocPtArm(0), fLambdaAssocPtRap(0), fLambdaAssocPtPhiEta(0), fLambdaMCResPhi(0), fAntiLambdaMCPt(0), fAntiLambdaMCPtRap(0), fAntiLambdaMCPtPhiEta(0), fAntiLambdaAssocPt(0), fAntiLambdaAssocPtArm(0), fAntiLambdaAssocPtRap(0), fAntiLambdaAssocPtPhiEta(0), fAntiLambdaMCResPhi(0),

  fHistArmenterosPodolanski(0), fHistArmPodBckg(0),
  
  fK0sMass(0), fK0sPtLtSB(0), fK0sPtvsEta(0), fK0sPtvsRap(0), fK0sEtaPhi(0), fK0sMassPtPhi(0), fK0sMassPtvsPtL(0), fK0sSiPtL(0), fK0sDaughtersPt(0), fK0sdPhiPtAssocPtL(0), fK0sDCADaugToPrimVtx(0), fK0sBckgDecLength(0), fK0sBckgDCADaugToPrimVtx(0), fK0sdEdxPosDaug(0), fK0sdEdxNegDaug(0), fK0sBckgEtaPhi(0), fK0sBckgPhiRadio(0), fK0sBckgDCANegDaugToPrimVtx(0), fK0sBckgDCAPosDaugToPrimVtx(0), fK0sMassCascade(0),

  fLambdaMass(0), fLambdaPtLtSB(0), fLambdaPtvsEta(0), fLambdaPtvsRap(0), fLambdaEtaPhi(0), fLambdaMassPtPhi(0), fLambdaMassPtvsPtL(0), fLambdaSiPtL(0), fLambdaDaughtersPt(0), fLambdadPhiPtAssocPtL(0), fLambdaDCADaugToPrimVtx(0), fLambdaBckgDecLength(0), fLambdaBckgDCADaugToPrimVtx(0), fLambdadEdxPosDaug(0), fLambdadEdxNegDaug(0), fLambdaBckgEtaPhi(0), fLambdaBckgPhiRadio(0), fLambdaBckgDCANegDaugToPrimVtx(0), fLambdaBckgDCAPosDaugToPrimVtx(0), fLambdaMassCascade(0),

 fAntiLambdaMass(0), fAntiLambdaPtLtSB(0), fAntiLambdaPtvsEta(0), fAntiLambdaPtvsRap(0), fAntiLambdaEtaPhi(0), fAntiLambdaMassPtPhi(0), fAntiLambdaMassPtvsPtL(0), fAntiLambdaSiPtL(0), fAntiLambdaDaughtersPt(0), fAntiLambdadPhiPtAssocPtL(0), fAntiLambdaDCADaugToPrimVtx(0), fAntiLambdaBckgDecLength(0), fAntiLambdaBckgDCADaugToPrimVtx(0), fAntiLambdadEdxPosDaug(0), fAntiLambdadEdxNegDaug(0), fAntiLambdaBckgEtaPhi(0), fAntiLambdaBckgPhiRadio(0), fAntiLambdaBckgDCANegDaugToPrimVtx(0), fAntiLambdaBckgDCAPosDaugToPrimVtx(0), fAntiLambdaMassCascade(0),

  fK0sPIDPosDaug(0), fK0sPIDNegDaug(0), fK0sBckgPIDPosDaug(0), fK0sBckgPIDNegDaug(0), fK0sPhiEtaPosDaug(0), fK0sPhiEtaNegDaug(0), fK0sBckgPhiEtaPosDaug(0), fK0sBckgPhiEtaNegDaug(0), fK0sDCAPosDaug(0), fK0sDCANegDaug(0), fK0sBckgDCAPosDaug(0), fK0sBckgDCANegDaug(0), fK0sDifPtPosDaug(0), fK0sDifPtNegDaug(0), fK0sBckgDifPtPosDaug(0), fK0sBckgDifPtNegDaug(0), fK0sDecayPos(0), fK0sBckgDecayPos(0), fK0sDecayVertex(0), fK0sBckgDecayVertex(0), fK0sDecayVertexZoom(0), fK0sBckgDecayVertexZoom(0), fK0sCPA(0), fK0sBckgCPA(0), fK0sDCAV0Daug(0), fK0sBckgDCAV0Daug(0),   fK0sNClustersTPC(0), fK0sBckgNClustersTPC(0), fK0sNClustersITSPos(0), fK0sNClustersITSNeg(0), fK0sBckgNClustersITSPos(0), fK0sBckgNClustersITSNeg(0),

  fLambdaPIDPosDaug(0), fLambdaPIDNegDaug(0), fLambdaBckgPIDPosDaug(0), fLambdaBckgPIDNegDaug(0), fLambdaPhiEtaPosDaug(0),fLambdaPhiEtaNegDaug(0), fLambdaBckgPhiEtaPosDaug(0),fLambdaBckgPhiEtaNegDaug(0), fLambdaDCAPosDaug(0),fLambdaDCANegDaug(0), fLambdaBckgDCAPosDaug(0), fLambdaBckgDCANegDaug(0), fLambdaDifPtPosDaug(0), fLambdaDifPtNegDaug(0), fLambdaBckgDifPtPosDaug(0), fLambdaBckgDifPtNegDaug(0), fLambdaDecayPos(0), fLambdaBckgDecayPos(0), fLambdaDecayVertex(0), fLambdaBckgDecayVertex(0), fLambdaDecayVertexZoom(0), fLambdaBckgDecayVertexZoom(0), fLambdaCPA(0), fLambdaBckgCPA(0), fLambdaDCAV0Daug(0), fLambdaBckgDCAV0Daug(0),  fLambdaNClustersTPC(0), fLambdaBckgNClustersTPC(0), fLambdaNClustersITSPos(0), fLambdaNClustersITSNeg(0), fLambdaBckgNClustersITSPos(0), fLambdaBckgNClustersITSNeg(0), 

  fAntiLambdaPIDPosDaug(0), fAntiLambdaPIDNegDaug(0), fAntiLambdaBckgPIDPosDaug(0), fAntiLambdaBckgPIDNegDaug(0), fAntiLambdaPhiEtaPosDaug(0),fAntiLambdaPhiEtaNegDaug(0), fAntiLambdaBckgPhiEtaPosDaug(0),fAntiLambdaBckgPhiEtaNegDaug(0), fAntiLambdaDCAPosDaug(0),fAntiLambdaDCANegDaug(0), fAntiLambdaBckgDCAPosDaug(0), fAntiLambdaBckgDCANegDaug(0), fAntiLambdaDifPtPosDaug(0), fAntiLambdaDifPtNegDaug(0), fAntiLambdaBckgDifPtPosDaug(0), fAntiLambdaBckgDifPtNegDaug(0), fAntiLambdaDecayPos(0), fAntiLambdaBckgDecayPos(0), fAntiLambdaDecayVertex(0), fAntiLambdaBckgDecayVertex(0), fAntiLambdaDecayVertexZoom(0), fAntiLambdaBckgDecayVertexZoom(0), fAntiLambdaCPA(0), fAntiLambdaBckgCPA(0), fAntiLambdaDCAV0Daug(0), fAntiLambdaBckgDCAV0Daug(0), fAntiLambdaNClustersTPC(0), fAntiLambdaBckgNClustersTPC(0), fAntiLambdaNClustersITSPos(0), fAntiLambdaNClustersITSNeg(0), fAntiLambdaBckgNClustersITSPos(0), fAntiLambdaBckgNClustersITSNeg(0)
  
{
  // Dummy Constructor

  for (Int_t i=0; i<kN1; i++){ 

    // K0s
    fK0sdPhidEtaMC[i] = 0;
    fK0sdPhidEtaMCCent[i] = 0;

    fK0sdPhidEtaPtL[i] = 0;
    fK0sdPhidEtaPtLCent[i] = 0;
    fK0sdPhidEtaPtLBckg[i] = 0;
    fK0sdPhidEtaPtLCentBckg[i] = 0;

    fK0sdPhidEtaPtL2[i] = 0;
    fK0sdPhidEtaPtLCent2[i] = 0;
    fK0sdPhidEtaPtLBckg2[i] = 0;
    fK0sdPhidEtaPtLCentBckg2[i] = 0;

    // Lambda
    fLambdadPhidEtaMC[i] = 0;
    fLambdadPhidEtaMCCent[i] = 0;

    fLambdadPhidEtaPtL[i] = 0;
    fLambdadPhidEtaPtLCent[i] = 0;
    fLambdadPhidEtaPtLBckg[i] = 0;
    fLambdadPhidEtaPtLCentBckg[i] = 0;
    
    fLambdadPhidEtaPtL2[i] = 0;
    fLambdadPhidEtaPtLCent2[i] = 0;
    fLambdadPhidEtaPtLBckg2[i] = 0;
    fLambdadPhidEtaPtLCentBckg2[i] = 0;

    // AntiLambda
    fAntiLambdadPhidEtaMC[i] = 0;
    fAntiLambdadPhidEtaMCCent[i] = 0;

    fAntiLambdadPhidEtaPtL[i] = 0;
    fAntiLambdadPhidEtaPtLCent[i] = 0;
    fAntiLambdadPhidEtaPtLBckg[i] = 0;
    fAntiLambdadPhidEtaPtLCentBckg[i] = 0;
    
    fAntiLambdadPhidEtaPtL2[i] = 0;
    fAntiLambdadPhidEtaPtLCent2[i] = 0;
    fAntiLambdadPhidEtaPtLBckg2[i] = 0;
    fAntiLambdadPhidEtaPtLCentBckg2[i] = 0;
  }
 
  // Constructor. Initialization of pointers
  DefineOutput(1, TList::Class());
  DefineOutput(2, TList::Class());
 

}

//___________________________________________________________________________________________

void AliAnalysisTaskLambdaOverK0sJets::UserCreateOutputObjects()
{ 
  // Creating the histograms that are needed for the output 
  
  fOutput = new TList(); 
  fOutput->SetOwner();

  fOutputQA = new TList(); 
  fOutputQA->SetOwner();

  // ====== General characteristics of the event and tracks ====== //

  // Counter for the number of events in each step:
  fEvents=new TH1F("fEvents","Number of events",14,-0.5,13.5);
  fEvents->GetXaxis()->SetBinLabel(1,"calls to UserExec()");
  fEvents->GetXaxis()->SetBinLabel(2,"AOD available");
  fEvents->GetXaxis()->SetBinLabel(3,"CINT1B");
  fEvents->GetXaxis()->SetBinLabel(4,"V0M Cent");
  fEvents->GetXaxis()->SetBinLabel(5,"Vtx > 3 part");
  fEvents->GetXaxis()->SetBinLabel(6,"|VtxZ| < 10 cm");
  fEvents->GetXaxis()->SetBinLabel(7,"Mult && Cent");
  fEvents->GetXaxis()->SetBinLabel(8,"Bad ID Trigger");
  fEvents->GetXaxis()->SetBinLabel(9,"V0 is LP");
  fEvents->GetXaxis()->SetBinLabel(10,"Trigger is V0 daug");
  fEvents->GetXaxis()->SetBinLabel(11,"Trigger is V0 daug && 2nd check");
  fEvents->GetXaxis()->SetBinLabel(12,"Triggered");
  fEvents->GetXaxis()->SetBinLabel(13,"NOT Triggered");
  fEvents->GetXaxis()->SetBinLabel(14,"V0 is LP in MC");
  fEvents->GetYaxis()->SetTitle("Counts"); 
  fOutput->Add(fEvents);

  // Centrality:
  fCentrality = new TH1F("fCentrality","Centrality;Centrality (%);Events",100,0.,100.);
  fOutput->Add(fCentrality);

  // Primary Vertex:
  fPrimaryVertexX = new TH1F("fPrimaryVertexX", "Primary Vertex Position X;Primary Vertex Position X (cm);Events",100,-0.5,0.5);
  fOutput->Add(fPrimaryVertexX);
  
  fPrimaryVertexY = new TH1F("fPrimaryVertexY", "Primary Vertex Position Y;Primary Vertex Position Y (cm);Events",100,-0.5,0.5);
  fOutput->Add(fPrimaryVertexY);
  
  fPrimaryVertexZ = new TH1F("fPrimaryVertexZ", "Primary Vertex Position Z;Primary Vertex Position Z (cm);Events",200,-20,20);
  fOutput->Add(fPrimaryVertexZ);

   // Multiplicity:
  fCentMult = new TH2F("fCentMult","Centrality vs Multiplicity;Centrality;Multiplicity",100,0.,100.,1100,0.,3300);
  fOutput->Add(fCentMult);

  // Energy loss:
  fdEdx = new TH2F("fdEdx","dE/dx;TPC Momentum (GeV/c);a.u.",50,0.2,3,50,0.,6.);
  fOutput->Add(fdEdx);

  fdEdxPid = new TH2F("fdEdxPid","dE/dx with PID (protons);TPC Momentum (GeV/c);a.u.",50,0.2,3,50,0.,6.);
  fOutput->Add(fdEdxPid);


  // ====== Trigger Particle characteristics ====== //

  // MC: Pt Trigger particle vs centrality:
  if(fIsMC){
  fTriggerMCPtCent = new TH3F("fTriggerMCPtCent","Trigger particle MC;p_{T} (GeV/c);#eta;centrality (%)",2*nbinPtLP,pMin,2*ptMaxLP,100,-1.,1.,100,0.,100.);
  fOutput->Add(fTriggerMCPtCent);
  }

  // Pt Trigger particle vs centrality:
  fTriggerPtCent = new TH3F("fTriggerPtCent","Trigger particle;p_{T} (GeV/c);centrality (%);Vertex Z (cm)",nbinPtLP,pMin,ptMaxLP,100,0.,100.,nbinsVtx,-10.,10.);
  fOutput->Add(fTriggerPtCent);

  // Phi vs pseudorapidity:
  fTriggerEtaPhi = new TH2F("fTriggerEtaPhi","Trigger particle;#phi (rad);#eta",nbinsPhi,0.,2.*TMath::Pi(),100,-1.,1.);
  fOutput->Add(fTriggerEtaPhi);
  
  // Check if Trigger particle comes from a V0 daughter:
  fCheckTriggerFromV0Daug = 
    new TH1F("fCheckTriggerFromV0Daug","Trigger particle from a V0 daughter",7,-2.5,4.5);
  fCheckTriggerFromV0Daug->GetXaxis()->SetTitle("Flag"); 
  fCheckTriggerFromV0Daug->GetXaxis()->SetBinLabel(2,"NOT V0 daug & 2nd check");
  fCheckTriggerFromV0Daug->GetXaxis()->SetBinLabel(3,"NOT V0 daug");
  fCheckTriggerFromV0Daug->GetXaxis()->SetBinLabel(4,"V0 daug");
  fCheckTriggerFromV0Daug->GetXaxis()->SetBinLabel(5,"V0 daug & V0 LP");
  fCheckTriggerFromV0Daug->GetXaxis()->SetBinLabel(6,"V0 daug & V0 LP & 2nd Check");
  fCheckTriggerFromV0Daug->GetYaxis()->SetTitle("Counts"); 
  fOutput->Add(fCheckTriggerFromV0Daug);
  
  fTriggerComingFromDaug = new TH1F("fTriggerComingFromDaug","Trigger particle from a V0 daughter;p_{T} (GeV/c);Counts",240, 0, 12);
  fOutput->Add(fTriggerComingFromDaug);

  fTriggerIsV0 = new TH1F("fTriggerIsV0","V0 candidate is a LP;p_{T} (GeV/c);Counts",nbinPtLP,pMin,ptMaxLP);
  fOutput->Add(fTriggerIsV0);

  fCheckIDTrigPtK0s = new TH3F("fCheckIDTrigPtK0s","K^{0}_{S}",200,-10.,10.,3,-0.5,2.5,100,1.,6.);
  fOutput->Add(fCheckIDTrigPtK0s);

  fCheckIDTrigPhiK0s = new TH3F("fCheckIDTrigPhiK0s","K^{0}_{S}",100,-2*TMath::Pi(),2*TMath::Pi(),3,-0.5,2.5,100,1.,6.);
  fOutput->Add(fCheckIDTrigPhiK0s);

  fCheckIDTrigPtLambda = new TH3F("fCheckIDTrigPtLambda","#Lambda",200,-10.,10.,3,-0.5,2.5,100.,1.,6.);
  fOutput->Add(fCheckIDTrigPtLambda);

  fCheckIDTrigPhiLambda  = new TH3F("fCheckIDTrigPhiLambda","#Lambda",100,-2*TMath::Pi(),2*TMath::Pi(),3,-0.5,2.5,100.,1.,6.);
  fOutput->Add(fCheckIDTrigPhiLambda);


  // ====== MC-true and  MC-Association information ====== //
  if(fIsMC){

  fInjectedParticles = new TH1F("fInjectedParticles","Injected particles",2,0.,2.);
  fInjectedParticles->GetXaxis()->SetBinLabel(1,"Injected");
  fInjectedParticles->GetXaxis()->SetBinLabel(2,"Natural");
  fInjectedParticles->GetYaxis()->SetTitle("Counts"); 
  fOutput->Add(fInjectedParticles);
    
  // K0s MC-true:
  fK0sMCPt       = new TH1F("fK0sMCPt", "K^{0}_{S} MC;p_{T} (GeV/c);Counts",nbins,pMin,pMax);
  fOutput->Add(fK0sMCPt);

  fK0sMCPtRap    = new TH3F("fK0sMCPtRap", "K^{0}_{S} MC",nbins,pMin,pMax,30,-1.5,1.5,100,0.,100.);
  fK0sMCPtRap->GetXaxis()->SetTitle("p_{T} (GeV/c)"); 
  fK0sMCPtRap->GetYaxis()->SetTitle("y"); 
  fK0sMCPtRap->GetZaxis()->SetTitle("centrality"); 
  fOutput->Add(fK0sMCPtRap);
  
  fK0sMCPtPhiEta    = new TH3F("fK0sMCPtPhiEta", "K^{0}_{S} MC",nbinsPhi,0.,2.*TMath::Pi(),30,-1.5,1.5,nbins,pMin,pMax);
  fK0sMCPtPhiEta->GetXaxis()->SetTitle("#phi (rad)"); 
  fK0sMCPtPhiEta->GetYaxis()->SetTitle("#eta"); 
  fK0sMCPtPhiEta->GetZaxis()->SetTitle("p_{T} (GeV/c)"); 
  fOutput->Add(fK0sMCPtPhiEta);
  
  // K0s MC-Association:
  fK0sAssocPt = 
    new TH1F("fK0sAssocPt","K^{0}_{S} Assoc: L_{T} vs p_{T}",nbins,pMin,pMax);
  fK0sAssocPt->GetXaxis()->SetTitle("p_{T} (GeV/c)"); 
  fOutput->Add(fK0sAssocPt);

  fK0sAssocPtArm = 
    new TH3F("fK0sAssocPtArm","K^{0}_{S} Assoc: p_{T} vs y vs centrality",nbins,pMin,pMax,30,-1.5,1.5,100,0.,100.);
  fK0sAssocPtArm->GetXaxis()->SetTitle("p_{T} (GeV/c)"); 
  fK0sAssocPtArm->GetYaxis()->SetTitle("y"); 
  fK0sAssocPtArm->GetZaxis()->SetTitle("centrality"); 
  fOutput->Add(fK0sAssocPtArm);

  fK0sAssocPtRap    = new TH3F("fK0sAssocPtRap","K^{0}_{S} Assoc",nbins,pMin,pMax,30,-1.5,1.5,100,0.,100.);
  fK0sAssocPtRap->GetXaxis()->SetTitle("p_{T} (GeV/c)"); 
  fK0sAssocPtRap->GetYaxis()->SetTitle("y"); 
  fK0sAssocPtRap->GetZaxis()->SetTitle("centrality");
  fOutput->Add(fK0sAssocPtRap);
  
  fK0sAssocPtPhiEta    = new TH3F("fK0sAssocPtPhiEta","K^{0}_{S} Assoc",nbinsPhi,0.,2.*TMath::Pi(),30,-1.5,1.5,nbins,pMin,pMax);
  fK0sAssocPtPhiEta->GetXaxis()->SetTitle("#phi"); 
  fK0sAssocPtPhiEta->GetYaxis()->SetTitle("#eta"); 
  fK0sAssocPtPhiEta->GetZaxis()->SetTitle("p_{T} (GeV/c)"); 
  fOutput->Add(fK0sAssocPtPhiEta);

  fK0sMCResPhi     = new TH3F("fK0sMCResPhi","K^{0}_{S} Assoc: #phi resolution; #phi_{MC}-#phi_{Rec};p_{T} (GeV/c); centrality",100,-0.5,0.5,nbins,pMin,pMax,100,0.,100.);
  fOutput->Add(fK0sMCResPhi);

  // Lambda MC-true: 
  fLambdaMCPt = new TH1F("fLambdaMCPt","#Lambda MC",nbins,pMin,pMax);
  fLambdaMCPt->GetXaxis()->SetTitle("p_{T} (GeV/c)"); 
  fOutput->Add(fLambdaMCPt);

  fLambdaMCPtRap = new TH3F("fLambdaMCPtRap","#Lambda MC",nbins,pMin,pMax,30,-1.5,1.5,100,0.,100.);
  fLambdaMCPtRap->GetXaxis()->SetTitle("p_{T} (GeV/c)"); 
  fLambdaMCPtRap->GetYaxis()->SetTitle("y"); 
  fLambdaMCPtRap->GetZaxis()->SetTitle("centrality");
  fOutput->Add(fLambdaMCPtRap);
  
  fLambdaMCPtPhiEta = new TH3F("fLambdaMCPtPhiEta","#Lambda MC",nbinsPhi,0.,2.*TMath::Pi(),30,-1.5,1.5,nbins,pMin,pMax);
  fLambdaMCPtPhiEta->GetXaxis()->SetTitle("#phi (rad)"); 
  fLambdaMCPtPhiEta->GetYaxis()->SetTitle("#eta"); 
  fLambdaMCPtPhiEta->GetZaxis()->SetTitle("p_{T} (GeV/c)"); 
  fOutput->Add(fLambdaMCPtPhiEta);
  

  // Lambda MC-Association:
  fLambdaAssocPt = 
    new TH1F("fLambdaAssocPt","#Lambda Assoc: L_{T} vs p_{T}",nbins,pMin,pMax);
  fLambdaAssocPt->GetXaxis()->SetTitle("p_{T} (GeV/c)"); 
  fOutput->Add(fLambdaAssocPt);

  /*
  fLambdaAssocPtArm = 
    new TH3F("fLambdaAssocPtArm","#Lambda Assoc: L_{T} vs p_{T}",nbins,pMin,pMax,2*nbins,lMin,lMax,100,0.,100.);
  fLambdaAssocPtArm->GetXaxis()->SetTitle("p_{T} (GeV/c)"); 
  fLambdaAssocPtArm->GetYaxis()->SetTitle("L_{T} (cm)"); 
  fLambdaAssocPtArm->GetZaxis()->SetTitle("centrality"); 
  fOutput->Add(fLambdaAssocPtArm);
  */

  fLambdaAssocPtRap = new TH3F("fLambdaAssocPtRap", "#Lambda Assoc",nbins,pMin,pMax,30,-1.5,1.5,100,0.,100.);
  fLambdaAssocPtRap->GetXaxis()->SetTitle("p_{T} (GeV/c)"); 
  fLambdaAssocPtRap->GetYaxis()->SetTitle("y"); 
  fLambdaAssocPtRap->GetZaxis()->SetTitle("centrality"); 
  fOutput->Add(fLambdaAssocPtRap);
  
  fLambdaAssocPtPhiEta = new TH3F("fLambdaAssocPtPhiEta", "#Lambda Assoc",nbinsPhi,0.,2.*TMath::Pi(),30,-1.5,1.5,nbins,pMin,pMax);
  fLambdaAssocPtPhiEta->GetXaxis()->SetTitle("#phi (rad)"); 
  fLambdaAssocPtPhiEta->GetYaxis()->SetTitle("#eta"); 
  fLambdaAssocPtPhiEta->GetZaxis()->SetTitle("p_{T} (GeV/c)"); 
  fOutput->Add(fLambdaAssocPtPhiEta);

  fLambdaMCResPhi     = new TH3F("fLambdaMCResPhi","#Lambda Assoc: #phi resolution;#phi_{MC}-#phi_{Rec};p_{T} (GeV/c); centrality",100,-0.5,0.5,nbins,pMin,pMax,100,0.,100.);
  fOutput->Add(fLambdaMCResPhi);
  
  // AntiLambda MC-true: 
  fAntiLambdaMCPt = new TH1F("fAntiLambdaMCPt","#bar{#Lambda} MC",nbins,pMin,pMax);
  fAntiLambdaMCPt->GetXaxis()->SetTitle("p_{T} (GeV/c)"); 
  fOutput->Add(fAntiLambdaMCPt);

  fAntiLambdaMCPtRap = new TH3F("fAntiLambdaMCPtRap","#bar{#Lambda} MC",nbins,pMin,pMax,30,-1.5,1.5,100,0.,100.);
  fAntiLambdaMCPtRap->GetXaxis()->SetTitle("p_{T} (GeV/c)"); 
  fAntiLambdaMCPtRap->GetYaxis()->SetTitle("y"); 
  fAntiLambdaMCPtRap->GetZaxis()->SetTitle("centrality");
  fOutput->Add(fAntiLambdaMCPtRap);
  
  fAntiLambdaMCPtPhiEta = new TH3F("fAntiLambdaMCPtPhiEta","#bar{#Lambda} MC",nbinsPhi,0.,2.*TMath::Pi(),30,-1.5,1.5,nbins,pMin,pMax);
  fAntiLambdaMCPtPhiEta->GetXaxis()->SetTitle("#phi (rad)"); 
  fAntiLambdaMCPtPhiEta->GetYaxis()->SetTitle("#eta"); 
  fAntiLambdaMCPtPhiEta->GetZaxis()->SetTitle("p_{T} (GeV/c)"); 
  fOutput->Add(fAntiLambdaMCPtPhiEta);




 // AntiLambda MC-Association:
  fAntiLambdaAssocPt = 
    new TH1F("fAntiLambdaAssocPt","#bar{#Lambda} Assoc: L_{T} vs p_{T}",nbins,pMin,pMax);
  fAntiLambdaAssocPt->GetXaxis()->SetTitle("p_{T} (GeV/c)"); 
  fOutput->Add(fAntiLambdaAssocPt);

  /*
  fAntiLambdaAssocPtArm = 
    new TH3F("fAntiLambdaAssocPtArm","#bar{#Lambda} Assoc: L_{T} vs p_{T}",nbins,pMin,pMax,2*nbins,lMin,lMax,100,0.,100.);
  fAntiLambdaAssocPtArm->GetXaxis()->SetTitle("p_{T} (GeV/c)"); 
  fAntiLambdaAssocPtArm->GetYaxis()->SetTitle("L_{T} (cm)"); 
  fAntiLambdaAssocPtArm->GetZaxis()->SetTitle("centrality"); 
  fOutput->Add(fAntiLambdaAssocPtArm);
  */

  fAntiLambdaAssocPtRap = new TH3F("fAntiLambdaAssocPtRap", "#bar{#Lambda} Assoc",nbins,pMin,pMax,30,-1.5,1.5,100,0.,100.);
  fAntiLambdaAssocPtRap->GetXaxis()->SetTitle("p_{T} (GeV/c)"); 
  fAntiLambdaAssocPtRap->GetYaxis()->SetTitle("y"); 
  fAntiLambdaAssocPtRap->GetZaxis()->SetTitle("centrality"); 
  fOutput->Add(fAntiLambdaAssocPtRap);
  
  fAntiLambdaAssocPtPhiEta = new TH3F("fAntiLambdaAssocPtPhiEta", "#bar{#Lambda} Assoc",nbinsPhi,0.,2.*TMath::Pi(),30,-1.5,1.5,nbins,pMin,pMax);
  fAntiLambdaAssocPtPhiEta->GetXaxis()->SetTitle("#phi (rad)"); 
  fAntiLambdaAssocPtPhiEta->GetYaxis()->SetTitle("#eta"); 
  fAntiLambdaAssocPtPhiEta->GetZaxis()->SetTitle("p_{T} (GeV/c)"); 
  fOutput->Add(fAntiLambdaAssocPtPhiEta);

  fAntiLambdaMCResPhi     = new TH3F("fAntiLambdaMCResPhi","#bar{#Lambda} Assoc: #phi resolution;#phi_{MC}-#phi_{Rec};p_{T} (GeV/c); centrality",100,-0.5,0.5,nbins,pMin,pMax,100,0.,100.);
  fOutput->Add(fAntiLambdaMCResPhi);






  }  

  // ====== Reconstruction information in AOD ====== //
  fHistArmenterosPodolanski  =
    new TH3F("fHistArmenterosPodolanski","Armenteros-Podolanski phase space;#alpha;p_{t} arm",
	     100,-1.0,1.0,50,0,0.5,7,-0.5,6.5);
  fHistArmenterosPodolanski->GetZaxis()->SetBinLabel(1,"K^{0}_{S} Inv. Mass Peak");
  fHistArmenterosPodolanski->GetZaxis()->SetBinLabel(2,"K^{0}_{S} Bckg");
  fHistArmenterosPodolanski->GetZaxis()->SetBinLabel(3,"#Lambda Inv. Mass Peak");
  fHistArmenterosPodolanski->GetZaxis()->SetBinLabel(4,"#Lambda Bckg");
  fHistArmenterosPodolanski->GetZaxis()->SetBinLabel(5,"#bar{#Lambda} Inv. Mass Peak");
  fHistArmenterosPodolanski->GetZaxis()->SetBinLabel(6,"#bar{#Lambda} Bckg");
  fOutput->Add(fHistArmenterosPodolanski);

  fHistArmPodBckg =
    new TH3F("fHistArmPodBckg","Background: Armenteros-Podolanski phase space;#alpha;p_{t} arm",
	     100,-1.0,1.0,50,0,0.5,4,-0.5,3.5);
  fHistArmPodBckg->GetZaxis()->SetBinLabel(1,"K^{0}_{S}: Trig events");
  fHistArmPodBckg->GetZaxis()->SetBinLabel(2,"#Lambda: Trig events");
  fHistArmPodBckg->GetZaxis()->SetBinLabel(3,"#bar{#Lambda}: Trig events");
  fOutput->Add(fHistArmPodBckg);
  
  // ****** K0s ******
  fK0sMass = 
    new TH3F("fK0sMass", "K^{0}_{s}: mass vs p_{T}",nbins/2,0.448,0.548,nbins,pMin,pMax,6,-0.5,5.5);
  fK0sMass->GetXaxis()->SetTitle("Mass (GeV/c^2)"); 
  fK0sMass->GetYaxis()->SetTitle("p_{T} (GeV/c)"); 
  fK0sMass->GetZaxis()->SetBinLabel(1,"All events");
  fK0sMass->GetZaxis()->SetBinLabel(2,"Central events");
  fK0sMass->GetZaxis()->SetBinLabel(3,"Arm-Pod Cut");
  fK0sMass->GetZaxis()->SetBinLabel(4,"Triggered events");
  fK0sMass->GetZaxis()->SetBinLabel(5,"Triggered central events");
  fOutput->Add(fK0sMass);

  fK0sPtLtSB = 
    new TH2F("fK0sPtLtSB","K^{0}_{s}: L_{T} vs p_{T}, side-band subtracted",
	     nbins,pMin,pMax,2*nbins,lMin,lMax);
  fK0sPtLtSB->GetXaxis()->SetTitle("p_{T} (GeV/c)"); 
  fK0sPtLtSB->GetYaxis()->SetTitle("L_{T} (cm)"); 
  fOutput->Add(fK0sPtLtSB);

  fK0sPtvsEta =
    new TH3F("fK0sPtvsEta","K^{0}_{s}: p_{T} vs #eta",nbins,pMin,pMax,30,-1.5,1.5,4,-0.5,3.5);
  fK0sPtvsEta->GetXaxis()->SetTitle("p_{T} (GeV/c)");
  fK0sPtvsEta->GetYaxis()->SetTitle("#eta");
  fK0sPtvsEta->GetZaxis()->SetBinLabel(1,"All events");
  fK0sPtvsEta->GetZaxis()->SetBinLabel(2,"Triggered events");
  fK0sPtvsEta->GetZaxis()->SetBinLabel(3,"All ev wi Inv Mass cut");
  fK0sPtvsEta->GetZaxis()->SetBinLabel(4,"Trig ev wi Inv Mass cut");
  fOutput->Add(fK0sPtvsEta);

  fK0sPtvsRap =
    new TH3F("fK0sPtvsRap","K^{0}_{s}: p_{T} vs y",nbins,pMin,pMax,30,-1.5,1.5,4,-0.5,3.5);
  fK0sPtvsRap->GetXaxis()->SetTitle("p_{T} (GeV/c)");
  fK0sPtvsRap->GetYaxis()->SetTitle("y");
  fK0sPtvsRap->GetZaxis()->SetBinLabel(1,"All events");
  fK0sPtvsRap->GetZaxis()->SetBinLabel(2,"Triggered events");
  fK0sPtvsRap->GetZaxis()->SetBinLabel(3,"All ev wi Inv Mass cut");
  fK0sPtvsRap->GetZaxis()->SetBinLabel(4,"Trig ev wi Inv Mass cut");
  fOutput->Add(fK0sPtvsRap);

  fK0sEtaPhi = 
    new TH2F("fK0sEtaPhi","K^{0}_{s}: #phi vs #eta",
	     nbinsPhi,0.,2.*TMath::Pi(),100,-1.,1.);
  fK0sEtaPhi->GetXaxis()->SetTitle("#phi (rad)"); 
  fK0sEtaPhi->GetYaxis()->SetTitle("#eta"); 
  fOutput->Add(fK0sEtaPhi);


  fK0sMassPtPhi  = 
    new TH3F("fK0sMassPtPhi","K^{0}_{s}: mass vs pt vs #phi",
	     nbins/2,0.448,0.548,nbins,pMin,pMax,nbinsPhi,0.,2.*TMath::Pi());
  fK0sMassPtPhi->GetXaxis()->SetTitle("Mass (GeV/c^2)"); 
  fK0sMassPtPhi->GetYaxis()->SetTitle("p_{T} (GeV/c)"); 
  fK0sMassPtPhi->GetZaxis()->SetTitle("#phi (rad)"); 
  fOutput->Add(fK0sMassPtPhi);


  // Correlations:
  fK0sMassPtvsPtL = 
    new TH3F("fK0sMassPtvsPtL", "K^{0}_{s}: mass vs p_{T} vs p_{T,l}",
	     nbins/2,0.448,0.548,
	     nbins,pMin,pMax,
	     nbinPtLP,pMin,ptMaxLP);
  fK0sMassPtvsPtL->GetXaxis()->SetTitle("Mass (GeV/c^2)"); 
  fK0sMassPtvsPtL->GetYaxis()->SetTitle("p_{T} (GeV/c)"); 
  fK0sMassPtvsPtL->GetZaxis()->SetTitle("p_{T,l} (GeV/c)"); 
  fOutput->Add(fK0sMassPtvsPtL);

  /*
  fK0sSiPtL =
    new TH3F("fK0sSiPtL","K^{0}_{s}: L_{T} vs p_{T} vs p_{t,L}, side-band subtracted",
	     nbins,pMin,pMax,2*nbins,lMin,lMax,
	     nbinPtLP,pMin,ptMaxLP);
  fK0sSiPtL->GetXaxis()->SetTitle("p_{T} (GeV/c)"); 
  fK0sSiPtL->GetYaxis()->SetTitle("L_{T} (cm)"); 
  fK0sSiPtL->GetZaxis()->SetTitle("p_{T,l} (GeV/c)"); 

  fOutput->Add(fK0sSiPtL);

  fK0sDaughtersPt =
    new TH2F("fK0sDaughtersPt","K^{0}_{s}: p_{T} of daughters",
	     nbins/2,pMin,pMax/2.,nbins/2,pMin,pMax/2.);
  fK0sDaughtersPt->GetXaxis()->SetTitle("Neg. Daught p_{T} (GeV/c)"); 
  fK0sDaughtersPt->GetYaxis()->SetTitle("Pos. Daught p_{T} (GeV/c)"); 
  fOutput->Add(fK0sDaughtersPt);
  */

  fK0sdPhiPtAssocPtL =
    new TH3F("fK0sdPhiPtAssocPtL","K^{0}_{s}: #Delta#phi vs  p_{T,a} vs p_{T,l}",
	     nbinsdPhi,-TMath::PiOver2(),3*TMath::PiOver2(),nbins,pMin,pMax,nbinPtLP,pMin,ptMaxLP);
  fK0sdPhiPtAssocPtL->GetXaxis()->SetTitle("#Delta#phi (rad)"); 
  fK0sdPhiPtAssocPtL->GetYaxis()->SetTitle("p_{T,a} (GeV/c)"); 
  fK0sdPhiPtAssocPtL->GetZaxis()->SetTitle("p_{T,l} (GeV/c)"); 
  fOutput->Add(fK0sdPhiPtAssocPtL);
  

  fK0sDCADaugToPrimVtx  
    = new TH3F("fK0sDCADaugToPrimVtx","K^{0}_{S} Bckg: dca daughter vs. p_{T,l}",
	       90,0.,3.3,90,0.,3.3,nbinPtLP,pMin,ptMaxLP);
  fK0sDCADaugToPrimVtx->GetXaxis()->SetTitle("DCA Pos daug (cm)"); 
  fK0sDCADaugToPrimVtx->GetYaxis()->SetTitle("DCA Neg daug (cm)"); 
  fK0sDCADaugToPrimVtx->GetZaxis()->SetTitle("p_{T,l} (GeV/c)"); 
  fOutput->Add(fK0sDCADaugToPrimVtx);

  //Float_t kVtxBins[] = {-10.,-7.,-4.,-2.,0.,2.,4.,7.,10.};
  char hNameHist[100];
  for(Int_t k=0;k<kN1;k++){

    // Monte-Carlo level:
    if(fIsMC){
    snprintf(hNameHist,100, "fK0sdPhidEtaMC_%.1f_%.1f",kPtBinV0[k],kPtBinV0[k+1]); 
    fK0sdPhidEtaMC[k] = new TH3F(hNameHist,"K^{0}_{S} MC: #Delta#phi vs #Delta#eta vs p_{T,l}",
				  nbinsdPhi,-TMath::PiOver2(),3*TMath::PiOver2(),
				  nbinsdEta,-2.,2.,
				  nbinsVtx,-10.,10.);
    fK0sdPhidEtaMC[k]->GetXaxis()->SetTitle("#Delta#phi (rad)"); 
    fK0sdPhidEtaMC[k]->GetYaxis()->SetTitle("#Delta#eta"); 
    fK0sdPhidEtaMC[k]->GetZaxis()->SetTitle("Vertex Z (cm)"); 
    fOutput->Add(fK0sdPhidEtaMC[k]);
  
    snprintf(hNameHist,100, "fK0sdPhidEtaMC_Cent_%.1f_%.1f",kPtBinV0[k],kPtBinV0[k+1]); 
    fK0sdPhidEtaMCCent[k] = new TH3F(hNameHist,"K^{0}_{S} MC: #Delta#phi vs #Delta#eta vs p_{T,l}",
				     nbinsdPhi,-TMath::PiOver2(),3*TMath::PiOver2(),
				     nbinsdEta,-2.,2.,
				     nbinsVtx,-10.,10.);
    fK0sdPhidEtaMCCent[k]->GetXaxis()->SetTitle("#Delta#phi (rad)");
    fK0sdPhidEtaMCCent[k]->GetYaxis()->SetTitle("#Delta#eta"); 
    fK0sdPhidEtaMCCent[k]->GetZaxis()->SetTitle("Vertex Z (cm)");
    fOutput->Add(fK0sdPhidEtaMCCent[k]);
    }

    // Reconstruction level:
    snprintf(hNameHist,100, "fK0sdPhidEtaPtL_%.1f_%.1f",kPtBinV0[k],kPtBinV0[k+1]); 
    fK0sdPhidEtaPtL[k] = new TH3F(hNameHist,"K^{0}_{S}: #Delta#phi vs #Delta#eta vs p_{T,l}",
				  nbinsdPhi,-TMath::PiOver2(),3*TMath::PiOver2(),
				  nbinsdEta,-2.,2.,
				  nbinsVtx,-10.,10.);
    fK0sdPhidEtaPtL[k]->GetXaxis()->SetTitle("#Delta#phi (rad)"); 
    fK0sdPhidEtaPtL[k]->GetYaxis()->SetTitle("#Delta#eta"); 
    fK0sdPhidEtaPtL[k]->GetZaxis()->SetTitle("Vertex Z (cm)"); 
    fOutput->Add(fK0sdPhidEtaPtL[k]);
  
    snprintf(hNameHist,100, "fK0sdPhidEtaPtL_Cent_%.1f_%.1f",kPtBinV0[k],kPtBinV0[k+1]); 
    fK0sdPhidEtaPtLCent[k] = new TH3F(hNameHist,"K^{0}_{S}: #Delta#phi vs #Delta#eta vs p_{T,l}",
				      nbinsdPhi,-TMath::PiOver2(),3*TMath::PiOver2(),
				      nbinsdEta,-2.,2.,
				      nbinsVtx,-10.,10.);
    fK0sdPhidEtaPtLCent[k]->GetXaxis()->SetTitle("#Delta#phi (rad)"); 
    fK0sdPhidEtaPtLCent[k]->GetYaxis()->SetTitle("#Delta#eta"); 
    fK0sdPhidEtaPtLCent[k]->GetZaxis()->SetTitle("Vertex Z (cm)");
    fOutput->Add(fK0sdPhidEtaPtLCent[k]);

    snprintf(hNameHist,100, "fK0sdPhidEtaPtL_Bckg_%.1f_%.1f",kPtBinV0[k],kPtBinV0[k+1]); 
    fK0sdPhidEtaPtLBckg[k] = new TH3F(hNameHist,"K^{0}_{S}: #Delta#phi vs #Delta#eta vs p_{T,l}",
				      nbinsdPhi,-TMath::PiOver2(),3*TMath::PiOver2(),
				      nbinsdEta,-2.,2.,
				      nbinsVtx,-10.,10.);
    fK0sdPhidEtaPtLBckg[k]->GetXaxis()->SetTitle("#Delta#phi (rad)"); 
    fK0sdPhidEtaPtLBckg[k]->GetYaxis()->SetTitle("#Delta#eta"); 
    fK0sdPhidEtaPtLBckg[k]->GetZaxis()->SetTitle("Vertex Z (cm)");
    fOutput->Add(fK0sdPhidEtaPtLBckg[k]);

    snprintf(hNameHist,100, "fK0sdPhidEtaPtL_Cent_Bckg_%.1f_%.1f",kPtBinV0[k],kPtBinV0[k+1]); 
    fK0sdPhidEtaPtLCentBckg[k] = new TH3F(hNameHist,"K^{0}_{S}: #Delta#phi vs #Delta#eta vs p_{T,l}", 
					  nbinsdPhi,-TMath::PiOver2(),3*TMath::PiOver2(),
					  nbinsdEta,-2.,2.,
					  nbinsVtx,-10.,10.);
    fK0sdPhidEtaPtLCentBckg[k]->GetXaxis()->SetTitle("#Delta#phi (rad)"); 
    fK0sdPhidEtaPtLCentBckg[k]->GetYaxis()->SetTitle("#Delta#eta"); 
    fK0sdPhidEtaPtLCentBckg[k]->GetZaxis()->SetTitle("Vertex Z (cm)");
    fOutput->Add(fK0sdPhidEtaPtLCentBckg[k]);

    // To be done with Armenteros Podolanski cut
    snprintf(hNameHist,100, "fK0sdPhidEtaPtL2_%.1f_%.1f",kPtBinV0[k],kPtBinV0[k+1]); 
    fK0sdPhidEtaPtL2[k] = new TH3F(hNameHist,"K^{0}_{S}: #Delta#phi vs #Delta#eta vs p_{T,l}",
				   nbinsdPhi,-TMath::PiOver2(),3*TMath::PiOver2(),
				   nbinsdEta,-2.,2.,
				   nbinsVtx,-10.,10.);
    fK0sdPhidEtaPtL2[k]->GetXaxis()->SetTitle("#Delta#phi (rad)"); 
    fK0sdPhidEtaPtL2[k]->GetYaxis()->SetTitle("#Delta#eta"); 
    fK0sdPhidEtaPtL2[k]->GetZaxis()->SetTitle("Vertex Z (cm)");
    fOutput->Add(fK0sdPhidEtaPtL2[k]);
  
    snprintf(hNameHist,100, "fK0sdPhidEtaPtL2_Cent_%.1f_%.1f",kPtBinV0[k],kPtBinV0[k+1]); 
    fK0sdPhidEtaPtLCent2[k] = new TH3F(hNameHist,"K^{0}_{S}: #Delta#phi vs #Delta#eta vs p_{T,l}",
				       nbinsdPhi,-TMath::PiOver2(),3*TMath::PiOver2(),
				       nbinsdEta,-2.,2.,
				       nbinsVtx,-10.,10.);
    fK0sdPhidEtaPtLCent2[k]->GetXaxis()->SetTitle("#Delta#phi (rad)"); 
    fK0sdPhidEtaPtLCent2[k]->GetYaxis()->SetTitle("#Delta#eta"); 
    fK0sdPhidEtaPtLCent2[k]->GetZaxis()->SetTitle("Vertex Z (cm)");
    fOutput->Add(fK0sdPhidEtaPtLCent2[k]);

    snprintf(hNameHist,100, "fK0sdPhidEtaPtL2_Bckg_%.1f_%.1f",kPtBinV0[k],kPtBinV0[k+1]); 
    fK0sdPhidEtaPtLBckg2[k] = new TH3F(hNameHist,"K^{0}_{S}: #Delta#phi vs #Delta#eta vs p_{T,l}",
				       nbinsdPhi,-TMath::PiOver2(),3*TMath::PiOver2(),
				       nbinsdEta,-2.,2.,
				       nbinsVtx,-10.,10.);
    fK0sdPhidEtaPtLBckg2[k]->GetXaxis()->SetTitle("#Delta#phi (rad)"); 
    fK0sdPhidEtaPtLBckg2[k]->GetYaxis()->SetTitle("#Delta#eta"); 
    fK0sdPhidEtaPtLBckg2[k]->GetZaxis()->SetTitle("Vertex Z (cm)");
    fOutput->Add(fK0sdPhidEtaPtLBckg2[k]);

    snprintf(hNameHist,100, "fK0sdPhidEtaPtL2_Cent_Bckg_%.1f_%.1f",kPtBinV0[k],kPtBinV0[k+1]); 
    fK0sdPhidEtaPtLCentBckg2[k] = new TH3F(hNameHist,"K^{0}_{S}: #Delta#phi vs #Delta#eta vs p_{T,l}", 
					   nbinsdPhi,-TMath::PiOver2(),3*TMath::PiOver2(),
					   nbinsdEta,-2.,2.,
					   nbinsVtx,-10.,10.);
    fK0sdPhidEtaPtLCentBckg2[k]->GetXaxis()->SetTitle("#Delta#phi (rad)"); 
    fK0sdPhidEtaPtLCentBckg2[k]->GetYaxis()->SetTitle("#Delta#eta"); 
    fK0sdPhidEtaPtLCentBckg2[k]->GetZaxis()->SetTitle("Vertex Z (cm)");
    fOutput->Add(fK0sdPhidEtaPtLCentBckg2[k]);
  }
  
  // Correlations (side-band):
  fK0sBckgDecLength
    = new TH2F("fK0sBckgDecLength","K^{0}_{S} Bckg: c#tau vs. p_{T,l}",
	       100,0.,15.,nbinPtLP,pMin,ptMaxLP);
  fK0sBckgDecLength->GetXaxis()->SetTitle("c#tau (cm)"); 
  fK0sBckgDecLength->GetYaxis()->SetTitle("p_{T,l} (GeV/c)"); 
  fOutput->Add(fK0sBckgDecLength);

  fK0sBckgDCADaugToPrimVtx  
    = new TH3F("fK0sBckgDCADaugToPrimVtx","K^{0}_{S} Bckg: dca daughter vs. p_{T,l}",
	       90,0.,3.3,90,0.,3.3,nbinPtLP,pMin,ptMaxLP);
  fK0sBckgDCADaugToPrimVtx->GetXaxis()->SetTitle("DCA Pos daug (cm)"); 
  fK0sBckgDCADaugToPrimVtx->GetYaxis()->SetTitle("DCA Neg daug (cm)"); 
  fK0sBckgDCADaugToPrimVtx->GetZaxis()->SetTitle("p_{T,l} (GeV/c)"); 
  fOutput->Add(fK0sBckgDCADaugToPrimVtx);
  
  fK0sBckgEtaPhi = 
    new TH2F("fK0sBckgEtaPhi","K^{0}_{s} Bckg: #phi vs #eta",
	     nbinsPhi,0.,2.*TMath::Pi(),100,-1.,1.);
  fK0sBckgEtaPhi->GetXaxis()->SetTitle("#phi (rad)"); 
  fK0sBckgEtaPhi->GetYaxis()->SetTitle("#eta"); 
  fOutput->Add(fK0sBckgEtaPhi);

  fK0sBckgPhiRadio
    = new TH2F("fK0sBckgPhiRadio","K^{0}_{S} Bckg: #phi vs l_{T}",
	       nbinsPhi,0.,2.*TMath::Pi(),2*nbins,lMin,lMax);
  fK0sBckgPhiRadio->GetXaxis()->SetTitle("#phi (rad)"); 
  fK0sBckgPhiRadio->GetYaxis()->SetTitle("l_{T} (cm)"); 
  fOutput->Add(fK0sBckgPhiRadio);
 
  fK0sBckgDCANegDaugToPrimVtx  
    = new TH2F("fK0sBckgDCANegDaugToPrimVtx","K^{0}_{S} Bckg: dca NegDaughter",
	       6,-0.5,5.5,90,0.,3.3);
  fK0sBckgDCANegDaugToPrimVtx->GetXaxis()->SetTitle("MC Production"); 
  fK0sBckgDCANegDaugToPrimVtx->GetXaxis()->SetBinLabel(1,"Rec");
  fK0sBckgDCANegDaugToPrimVtx->GetXaxis()->SetBinLabel(2,"Primary");
  fK0sBckgDCANegDaugToPrimVtx->GetXaxis()->SetBinLabel(3,"Weak Decay");
  fK0sBckgDCANegDaugToPrimVtx->GetXaxis()->SetBinLabel(4,"Gamma conv.");
  fK0sBckgDCANegDaugToPrimVtx->GetXaxis()->SetBinLabel(5,"Unidentified mother");
  fK0sBckgDCANegDaugToPrimVtx->GetXaxis()->SetBinLabel(6,"Other");
  fK0sBckgDCANegDaugToPrimVtx->GetYaxis()->SetTitle("DCA Neg Daug (cm)"); 
  fOutput->Add(fK0sBckgDCANegDaugToPrimVtx);

  fK0sBckgDCAPosDaugToPrimVtx  
    = new TH2F("fK0sBckgDCAPosDaugToPrimVtx","K^{0}_{S} Bckg: dca PosDaughter",
	       6,-0.5,5.5,90,0.,3.3);
  fK0sBckgDCAPosDaugToPrimVtx->GetXaxis()->SetTitle("MC Production"); 
  fK0sBckgDCAPosDaugToPrimVtx->GetXaxis()->SetBinLabel(1,"Rec");
  fK0sBckgDCAPosDaugToPrimVtx->GetXaxis()->SetBinLabel(2,"Primary");
  fK0sBckgDCAPosDaugToPrimVtx->GetXaxis()->SetBinLabel(3,"Weak Decay");
  fK0sBckgDCAPosDaugToPrimVtx->GetXaxis()->SetBinLabel(4,"Gamma conv.");
  fK0sBckgDCAPosDaugToPrimVtx->GetXaxis()->SetBinLabel(5,"Unidentified mother");
  fK0sBckgDCAPosDaugToPrimVtx->GetXaxis()->SetBinLabel(6,"Other");
  fK0sBckgDCAPosDaugToPrimVtx->GetYaxis()->SetTitle("DCA Pos Daug (cm)"); 
  fOutput->Add(fK0sBckgDCAPosDaugToPrimVtx);
        
  fK0sMassCascade
    = new TH2F("fK0sMassCascade","Cascade Reconstruction in K^{0}_{S} Bckg",650, 1.2, 2.5,4,0.5,4.5);
  fK0sMassCascade->GetXaxis()->SetTitle("Invariant Mass (GeV/c^{2})");
  fK0sMassCascade->GetYaxis()->SetTitle("Cascade type");
  fOutput->Add(fK0sMassCascade);


  // ****** Lambda ******
  fLambdaMass = 
    new TH3F("fLambdaMass","Mass vs p_{T} for \\Lambda",nbins,1.065,1.165,nbins,pMin,pMax,6,-0.5,5.5);
  fLambdaMass->GetXaxis()->SetTitle("Mass (GeV/c^2)");
  fLambdaMass->GetYaxis()->SetTitle("p_{T} (GeV/c)"); 
  fLambdaMass->GetZaxis()->SetBinLabel(1,"All events");
  fLambdaMass->GetZaxis()->SetBinLabel(2,"Central events");
  fLambdaMass->GetZaxis()->SetBinLabel(3,"Arm-Pod cut");
  fLambdaMass->GetZaxis()->SetBinLabel(4,"Triggered events");
  fLambdaMass->GetZaxis()->SetBinLabel(5,"Triggered central events");
  fOutput->Add(fLambdaMass);
  
  fLambdaPtLtSB = 
    new TH2F("fLambdaPtLtSB","L_{T} vs p_{T} for \\Lambda, side-band subtructed",
	     nbins,pMin,pMax,nbins,2*lMin,lMax);
  fLambdaPtLtSB->GetXaxis()->SetTitle("p_{T} (GeV/c)"); 
  fLambdaPtLtSB->GetYaxis()->SetTitle("L_{T} (cm)"); 
  fOutput->Add(fLambdaPtLtSB);

  fLambdaPtvsEta =
    new TH3F("fLambdaPtvsEta","\\Lambda: p_{T} vs #eta",nbins,pMin,pMax,30,-1.5,1.5,4,-0.5,3.5);
  fLambdaPtvsEta->GetXaxis()->SetTitle("p_{T} (GeV/c)");
  fLambdaPtvsEta->GetYaxis()->SetTitle("#eta");
  fK0sPtvsEta->GetXaxis()->SetTitle("p_{T} (GeV/c)");
  fLambdaPtvsEta->GetYaxis()->SetTitle("#eta");
  fLambdaPtvsEta->GetZaxis()->SetBinLabel(1,"All events");
  fLambdaPtvsEta->GetZaxis()->SetBinLabel(2,"Triggered events");
  fLambdaPtvsEta->GetZaxis()->SetBinLabel(3,"All ev wi Inv Mass cut");
  fLambdaPtvsEta->GetZaxis()->SetBinLabel(4,"Trig ev wi Inv Mass cut");
  fOutput->Add(fLambdaPtvsEta);

  fLambdaPtvsRap =
    new TH3F("fLambdaPtvsRap","\\Lambda: p_{T} vs y",nbins,pMin,pMax,30,-1.5,1.5,4,-0.5,3.5);
  fLambdaPtvsRap->GetXaxis()->SetTitle("p_{T} (GeV/c)");
  fLambdaPtvsRap->GetYaxis()->SetTitle("y");
  fLambdaPtvsRap->GetYaxis()->SetTitle("#eta");
  fLambdaPtvsRap->GetZaxis()->SetBinLabel(1,"All events");
  fLambdaPtvsRap->GetZaxis()->SetBinLabel(2,"Triggered events");
  fLambdaPtvsRap->GetZaxis()->SetBinLabel(3,"All ev wi Inv Mass cut");
  fLambdaPtvsRap->GetZaxis()->SetBinLabel(4,"Trig ev wi Inv Mass cut");
  fOutput->Add(fLambdaPtvsRap);

  fLambdaEtaPhi =
    new TH2F("fLambdaEtaPhi","#phi vs #eta for #Lambda",
	     nbinsPhi,0.,2.*TMath::Pi(),100,-1.,1.);
  fLambdaEtaPhi->GetXaxis()->SetTitle("#phi (rad)"); 
  fLambdaEtaPhi->GetYaxis()->SetTitle("#eta"); 
  fOutput->Add(fLambdaEtaPhi);


  fLambdaMassPtPhi  = 
    new TH3F("fLambdaMassPtPhi","#Lambda: mass vs pt vs #phi",
	     nbins,1.065,1.165,nbins,pMin,pMax,nbinsPhi,0.,2.*TMath::Pi());
  fLambdaMassPtPhi->GetXaxis()->SetTitle("Mass (GeV/c^2)"); 
  fLambdaMassPtPhi->GetYaxis()->SetTitle("p_{T} (GeV/c)"); 
  fLambdaMassPtPhi->GetZaxis()->SetTitle("#phi (rad)"); 
  fOutput->Add(fLambdaMassPtPhi);

  // Correlations:
  fLambdaMassPtvsPtL = 
    new TH3F("fLambdaMassPtvsPtL", "#Lambda: mass vs p_{T} vs p_{T,l}",
	     nbins,1.065,1.165,
	     nbins,pMin,pMax,
	     nbinPtLP,pMin,ptMaxLP);
  fLambdaMassPtvsPtL->GetXaxis()->SetTitle("Mass (GeV/c^2)"); 
  fLambdaMassPtvsPtL->GetYaxis()->SetTitle("p_{T} (GeV/c)"); 
  fLambdaMassPtvsPtL->GetZaxis()->SetTitle("p_{T,l} (GeV/c)"); 
  fOutput->Add(fLambdaMassPtvsPtL);

  /*
  fLambdaSiPtL =
    new TH3F("fLambdaSiPtL","#Lambda: L_{T} vs p_{T} vs p_{t,L}, side-band subtracted",
	     nbins,pMin,pMax,2*nbins,lMin,lMax,
	     nbinPtLP,pMin,ptMaxLP);
  fLambdaSiPtL->GetXaxis()->SetTitle("p_{T} (GeV/c)"); 
  fLambdaSiPtL->GetYaxis()->SetTitle("L_{T} (cm)"); 
  fLambdaSiPtL->GetZaxis()->SetTitle("p_{T,l} (GeV/c)"); 
  fOutput->Add(fLambdaSiPtL);

  fLambdaDaughtersPt =
    new TH2F("fLambdaDaughtersPt","#Lambda: p_{T} of daughters",
	     nbins/2,pMin,pMax/2.,nbins/2,pMin,pMax/2.);
  fLambdaDaughtersPt->GetXaxis()->SetTitle("Neg. Daught p_{T} (GeV/c)"); 
  fLambdaDaughtersPt->GetYaxis()->SetTitle("Pos. Daught p_{T} (GeV/c)"); 
  fOutput->Add(fLambdaDaughtersPt);
  */

  fLambdadPhiPtAssocPtL =
    new TH3F("fLambdadPhiPtAssocPtL","#Lambda: #Delta#phi vs  p_{T,a} vs p_{T,l}",
	     nbinsPhi,-TMath::PiOver2(),3*TMath::PiOver2(),nbins,pMin,pMax,nbinPtLP,pMin,ptMaxLP);
  fLambdadPhiPtAssocPtL->GetXaxis()->SetTitle("#Delta#phi (rad)"); 
  fLambdadPhiPtAssocPtL->GetYaxis()->SetTitle("p_{T,a} (GeV/c)"); 
  fLambdadPhiPtAssocPtL->GetZaxis()->SetTitle("p_{T,l} (GeV/c)"); 
  fOutput->Add(fLambdadPhiPtAssocPtL);

  fLambdaDCADaugToPrimVtx  
    = new TH3F("fLambdaDCADaugToPrimVtx","#Lambda Bckg: dca daughter vs. p_{T,l}",
	       90,0.,3.3,90,0.,3.3,nbinPtLP,pMin,ptMaxLP);
  fLambdaDCADaugToPrimVtx->GetXaxis()->SetTitle("DCA Pos daug (cm)"); 
  fLambdaDCADaugToPrimVtx->GetYaxis()->SetTitle("DCA Neg daug (cm)"); 
  fLambdaDCADaugToPrimVtx->GetZaxis()->SetTitle("p_{T,l} (GeV/c)"); 
  fOutput->Add(fLambdaDCADaugToPrimVtx);


  for(Int_t k=0;k<kN1;k++){

    // Monte-Carlo level:
    if(fIsMC){
    snprintf(hNameHist,100, "fLambdadPhidEtaMC_%.1f_%.1f",kPtBinV0[k],kPtBinV0[k+1]); 
    fLambdadPhidEtaMC[k] = new TH3F(hNameHist,"#Lambda MC: #Delta#phi vs #Delta#eta vs p_{T,l}",
				  nbinsdPhi,-TMath::PiOver2(),3*TMath::PiOver2(),
				  nbinsdEta,-2.,2.,
				  nbinsVtx,-10.,10.);
    fLambdadPhidEtaMC[k]->GetXaxis()->SetTitle("#Delta#phi (rad)"); 
    fLambdadPhidEtaMC[k]->GetYaxis()->SetTitle("#Delta#eta"); 
    fLambdadPhidEtaMC[k]->GetZaxis()->SetTitle("Vertex Z (cm)"); 
    fOutput->Add(fLambdadPhidEtaMC[k]);
  
    snprintf(hNameHist,100, "fLambdadPhidEtaMC_Cent_%.1f_%.1f",kPtBinV0[k],kPtBinV0[k+1]); 
    fLambdadPhidEtaMCCent[k] = new TH3F(hNameHist,"#Lambda MC: #Delta#phi vs #Delta#eta vs p_{T,l}",
				      nbinsdPhi,-TMath::PiOver2(),3*TMath::PiOver2(),
				      nbinsdEta,-2.,2.,
				      nbinsVtx,-10.,10.);
    fLambdadPhidEtaMCCent[k]->GetXaxis()->SetTitle("#Delta#phi (rad)"); 
    fLambdadPhidEtaMCCent[k]->GetYaxis()->SetTitle("#Delta#eta"); 
    fLambdadPhidEtaMCCent[k]->GetZaxis()->SetTitle("Vertex Z (cm)");
    fOutput->Add(fLambdadPhidEtaMCCent[k]);
    }

    // Reconstruction level:
    snprintf(hNameHist,100, "fLambdadPhidEtaPtL_%.1f_%.1f",kPtBinV0[k],kPtBinV0[k+1]); 
    fLambdadPhidEtaPtL[k] = new TH3F(hNameHist,"#Lambda: #Delta#phi vs #Delta#eta vs p_{T,l}",
				     nbinsdPhi,-TMath::PiOver2(),3*TMath::PiOver2(),
				     nbinsdEta,-2.,2.,
				     nbinsVtx,-10.,10.);
    fLambdadPhidEtaPtL[k]->GetXaxis()->SetTitle("#Delta#phi (rad)"); 
    fLambdadPhidEtaPtL[k]->GetYaxis()->SetTitle("#Delta#eta"); 
    fLambdadPhidEtaPtL[k]->GetZaxis()->SetTitle("Vertex Z (cm)");
    fOutput->Add(fLambdadPhidEtaPtL[k]);
  
    snprintf(hNameHist,100, "fLambdadPhidEtaPtL_Cent_%.1f_%.1f",kPtBinV0[k],kPtBinV0[k+1]); 
    fLambdadPhidEtaPtLCent[k] = new TH3F(hNameHist,"#Lambda: #Delta#phi  vs #Delta#eta vs p_{T,l}",
					 nbinsdPhi,-TMath::PiOver2(),3*TMath::PiOver2(),
					 nbinsdEta,-2.,2.,
					 nbinsVtx,-10.,10.);
    fLambdadPhidEtaPtLCent[k]->GetXaxis()->SetTitle("#Delta#phi (rad)"); 
    fLambdadPhidEtaPtLCent[k]->GetYaxis()->SetTitle("#Delta#eta"); 
    fLambdadPhidEtaPtLCent[k]->GetZaxis()->SetTitle("Vertex Z (cm)");
    fOutput->Add(fLambdadPhidEtaPtLCent[k]);

    snprintf(hNameHist,100, "fLambdadPhidEtaPtL_Bckg_%.1f_%.1f",kPtBinV0[k],kPtBinV0[k+1]); 
    fLambdadPhidEtaPtLBckg[k] = new TH3F(hNameHist,"#Lambda: #Delta#phi  vs #Delta#eta vs p_{T,l}",
					 nbinsdPhi,-TMath::PiOver2(),3*TMath::PiOver2(),
					 nbinsdEta,-2.,2.,
					 nbinsVtx,-10.,10.);
    fLambdadPhidEtaPtLBckg[k]->GetXaxis()->SetTitle("#Delta#phi (rad)"); 
    fLambdadPhidEtaPtLBckg[k]->GetYaxis()->SetTitle("#Delta#eta"); 
    fLambdadPhidEtaPtLBckg[k]->GetZaxis()->SetTitle("Vertex Z (cm)");
    fOutput->Add(fLambdadPhidEtaPtLBckg[k]);

    snprintf(hNameHist,100, "fLambdadPhidEtaPtL_Cent_Bckg_%.1f_%.1f",kPtBinV0[k],kPtBinV0[k+1]); 
    fLambdadPhidEtaPtLCentBckg[k] = new TH3F(hNameHist,"#Lambda: #Delta#phi  vs #Delta#eta vs p_{T,l}",
					     nbinsdPhi,-TMath::PiOver2(),3*TMath::PiOver2(),
					     nbinsdEta,-2.,2.,
					     nbinsVtx,-10.,10.);
    fLambdadPhidEtaPtLCentBckg[k]->GetXaxis()->SetTitle("#Delta#phi (rad)"); 
    fLambdadPhidEtaPtLCentBckg[k]->GetYaxis()->SetTitle("#Delta#eta"); 
    fLambdadPhidEtaPtLCentBckg[k]->GetZaxis()->SetTitle("Vertex Z (cm)");
    fOutput->Add(fLambdadPhidEtaPtLCentBckg[k]);


    //  To be used when the Armenteros Podolanski plot is applied
    snprintf(hNameHist,100, "fLambdadPhidEtaPtL2_%.1f_%.1f",kPtBinV0[k],kPtBinV0[k+1]); 
    fLambdadPhidEtaPtL2[k] = new TH3F(hNameHist,"#Lambda: #Delta#phi vs #Delta#eta vs p_{T,l}",
				      nbinsdPhi,-TMath::PiOver2(),3*TMath::PiOver2(),
				      nbinsdEta,-2.,2.,
				      nbinsVtx,-10.,10.);
    fLambdadPhidEtaPtL2[k]->GetXaxis()->SetTitle("#Delta#phi (rad)"); 
    fLambdadPhidEtaPtL2[k]->GetYaxis()->SetTitle("#Delta#eta"); 
    fLambdadPhidEtaPtL2[k]->GetZaxis()->SetTitle("Vertex Z (cm)");
    fOutput->Add(fLambdadPhidEtaPtL2[k]);
  
    snprintf(hNameHist,100, "fLambdadPhidEtaPtL2_Cent_%.1f_%.1f",kPtBinV0[k],kPtBinV0[k+1]); 
    fLambdadPhidEtaPtLCent2[k] = new TH3F(hNameHist,"#Lambda: #Delta#phi  vs #Delta#eta vs p_{T,l}",
					  nbinsdPhi,-TMath::PiOver2(),3*TMath::PiOver2(),
					  nbinsdEta,-2.,2.,
					  nbinsVtx,-10.,10.);
    fLambdadPhidEtaPtLCent2[k]->GetXaxis()->SetTitle("#Delta#phi (rad)"); 
    fLambdadPhidEtaPtLCent2[k]->GetYaxis()->SetTitle("#Delta#eta"); 
    fLambdadPhidEtaPtLCent2[k]->GetZaxis()->SetTitle("Vertex Z (cm)");
    fOutput->Add(fLambdadPhidEtaPtLCent2[k]);

    snprintf(hNameHist,100, "fLambdadPhidEtaPtL2_Bckg_%.1f_%.1f",kPtBinV0[k],kPtBinV0[k+1]); 
    fLambdadPhidEtaPtLBckg2[k] = new TH3F(hNameHist,"#Lambda: #Delta#phi  vs #Delta#eta vs p_{T,l}",
					  nbinsdPhi,-TMath::PiOver2(),3*TMath::PiOver2(),
					  nbinsdEta,-2.,2.,
					  nbinsVtx,-10.,10.);
    fLambdadPhidEtaPtLBckg2[k]->GetXaxis()->SetTitle("#Delta#phi (rad)"); 
    fLambdadPhidEtaPtLBckg2[k]->GetYaxis()->SetTitle("#Delta#eta"); 
    fLambdadPhidEtaPtLBckg2[k]->GetZaxis()->SetTitle("Vertex Z (cm)");
    fOutput->Add(fLambdadPhidEtaPtLBckg2[k]);

    snprintf(hNameHist,100, "fLambdadPhidEtaPtL2_Cent_Bckg_%.1f_%.1f",kPtBinV0[k],kPtBinV0[k+1]); 
    fLambdadPhidEtaPtLCentBckg2[k] = new TH3F(hNameHist,"#Lambda: #Delta#phi  vs #Delta#eta vs p_{T,l}",
					      nbinsdPhi,-TMath::PiOver2(),3*TMath::PiOver2(),
					      nbinsdEta,-2.,2.,
					      nbinsVtx,-10.,10.);
    fLambdadPhidEtaPtLCentBckg2[k]->GetXaxis()->SetTitle("#Delta#phi (rad)"); 
    fLambdadPhidEtaPtLCentBckg2[k]->GetYaxis()->SetTitle("#Delta#eta"); 
    fLambdadPhidEtaPtLCentBckg2[k]->GetZaxis()->SetTitle("Vertex Z (cm)");
    fOutput->Add(fLambdadPhidEtaPtLCentBckg2[k]);

  }
  

  // Correlations (side-band):
  fLambdaBckgDecLength
    = new TH2F("fLambdaBckgDecLength","#Lambda Bckg: c#tau vs. p_{T,l}",
	       100,0.,25.,nbinPtLP,pMin,ptMaxLP);
  fLambdaBckgDecLength->GetXaxis()->SetTitle("c#tau (cm)"); 
  fLambdaBckgDecLength->GetYaxis()->SetTitle("p_{T,l} (GeV/c)"); 
  fOutput->Add(fLambdaBckgDecLength);
  
  fLambdaBckgDCADaugToPrimVtx  
    = new TH3F("fLambdaBckgDCADaugToPrimVtx","#Lambda Bckg: dca daughter vs. p_{T,l}",
	       90,0.,3.3,90,0.,3.3,nbinPtLP,pMin,ptMaxLP);
  fLambdaBckgDCADaugToPrimVtx->GetXaxis()->SetTitle("DCA Pos daug (cm)"); 
  fLambdaBckgDCADaugToPrimVtx->GetYaxis()->SetTitle("DCA Neg daug (cm)"); 
  fLambdaBckgDCADaugToPrimVtx->GetZaxis()->SetTitle("p_{T,l} (GeV/c)"); 
  fOutput->Add(fLambdaBckgDCADaugToPrimVtx);
  
  fLambdaBckgEtaPhi = 
    new TH2F("fLambdaBckgEtaPhi","#Lambda Bckg: #phi vs #eta",
	     nbinsPhi,0.,2.*TMath::Pi(),100,-1.,1.);
  fLambdaBckgEtaPhi->GetXaxis()->SetTitle("#phi (rad)"); 
  fLambdaBckgEtaPhi->GetYaxis()->SetTitle("#eta"); 
  fOutput->Add(fLambdaBckgEtaPhi);
    
  fLambdaBckgPhiRadio
    = new TH2F("fLambdaBckgPhiRadio","#Lambda Bckg: #phi vs l_{T}",
	       nbinsPhi,0.,2.*TMath::Pi(),2*nbins,lMin,lMax);
  fLambdaBckgPhiRadio->GetXaxis()->SetTitle("#phi (rad)"); 
  fLambdaBckgPhiRadio->GetYaxis()->SetTitle("l_{T} (cm)"); 
  fOutput->Add(fLambdaBckgPhiRadio);


  fLambdaBckgDCANegDaugToPrimVtx  
    = new TH2F("fLambdaBckgDCANegDaugToPrimVtx","#Lambda Bckg: dca NegDaughter",
	       6,-0.5,5.5,90,0.,3.3);
  fLambdaBckgDCANegDaugToPrimVtx->GetXaxis()->SetTitle("MC Production"); 
  fLambdaBckgDCANegDaugToPrimVtx->GetXaxis()->SetBinLabel(1,"Rec");
  fLambdaBckgDCANegDaugToPrimVtx->GetXaxis()->SetBinLabel(2,"Primary");
  fLambdaBckgDCANegDaugToPrimVtx->GetXaxis()->SetBinLabel(3,"Weak Decay");
  fLambdaBckgDCANegDaugToPrimVtx->GetXaxis()->SetBinLabel(4,"Gamma conv.");
  fLambdaBckgDCANegDaugToPrimVtx->GetXaxis()->SetBinLabel(5,"Unidentified mother");
  fLambdaBckgDCANegDaugToPrimVtx->GetXaxis()->SetBinLabel(6,"Other");
  fLambdaBckgDCANegDaugToPrimVtx->GetYaxis()->SetTitle("DCA Neg Daug (cm)"); 
  fOutput->Add(fLambdaBckgDCANegDaugToPrimVtx);


  fLambdaBckgDCAPosDaugToPrimVtx  
    = new TH2F("fLambdaBckgDCAPosDaugToPrimVtx","#Lambda Bckg: dca PosDaughter",
	       6,-0.5,5.5,90,0.,3.3);
  fLambdaBckgDCAPosDaugToPrimVtx->GetXaxis()->SetTitle("MC Production"); 
  fLambdaBckgDCAPosDaugToPrimVtx->GetXaxis()->SetBinLabel(1,"Rec");
  fLambdaBckgDCAPosDaugToPrimVtx->GetXaxis()->SetBinLabel(2,"Primary");
  fLambdaBckgDCAPosDaugToPrimVtx->GetXaxis()->SetBinLabel(3,"Weak Decay");
  fLambdaBckgDCAPosDaugToPrimVtx->GetXaxis()->SetBinLabel(4,"Gamma conv.");
  fLambdaBckgDCAPosDaugToPrimVtx->GetXaxis()->SetBinLabel(5,"Unidentified mother");
  fLambdaBckgDCAPosDaugToPrimVtx->GetXaxis()->SetBinLabel(6,"Other");
  fLambdaBckgDCAPosDaugToPrimVtx->GetYaxis()->SetTitle("DCA Pos Daug (cm)"); 
  fOutput->Add(fLambdaBckgDCAPosDaugToPrimVtx);


  fLambdaMassCascade
    = new TH2F("fLambdaMassCascade","Cascade Reconstruction in #Lambda Bckg",650, 1.2, 2.5,4,0.5,4.5);
  fLambdaMassCascade->GetXaxis()->SetTitle("Invariant Mass (GeV/c^{2})");
  fLambdaMassCascade->GetYaxis()->SetTitle("Cascade type");
  fOutput->Add(fLambdaMassCascade);


  // ****** AntiLambda ******
  fAntiLambdaMass = 
    new TH3F("fAntiLambdaMass","Mass vs p_{T} for #bar{#Lambda}",nbins,1.065,1.165,nbins,pMin,pMax,6,-0.5,5.5);
  fAntiLambdaMass->GetXaxis()->SetTitle("Mass (GeV/c^2)");
  fAntiLambdaMass->GetYaxis()->SetTitle("p_{T} (GeV/c)"); 
  fAntiLambdaMass->GetZaxis()->SetBinLabel(1,"All events");
  fAntiLambdaMass->GetZaxis()->SetBinLabel(2,"Central events");
  fAntiLambdaMass->GetZaxis()->SetBinLabel(3,"Arm-Pod cut");
  fAntiLambdaMass->GetZaxis()->SetBinLabel(4,"Triggered events");
  fAntiLambdaMass->GetZaxis()->SetBinLabel(5,"Triggered central events");
  fOutput->Add(fAntiLambdaMass);
  
  fAntiLambdaPtLtSB = 
    new TH2F("fAntiLambdaPtLtSB","L_{T} vs p_{T} for #bar{#Lambda}, side-band subtructed",
	     nbins,pMin,pMax,nbins,2*lMin,lMax);
  fAntiLambdaPtLtSB->GetXaxis()->SetTitle("p_{T} (GeV/c)"); 
  fAntiLambdaPtLtSB->GetYaxis()->SetTitle("L_{T} (cm)"); 
  fOutput->Add(fAntiLambdaPtLtSB);

  fAntiLambdaPtvsEta =
    new TH3F("fAntiLambdaPtvsEta","#bar{#Lambda}: p_{T} vs #eta",nbins,pMin,pMax,30,-1.5,1.5,4,-0.5,3.5);
  fAntiLambdaPtvsEta->GetXaxis()->SetTitle("p_{T} (GeV/c)");
  fAntiLambdaPtvsEta->GetYaxis()->SetTitle("#eta");
  fK0sPtvsEta->GetXaxis()->SetTitle("p_{T} (GeV/c)");
  fAntiLambdaPtvsEta->GetYaxis()->SetTitle("#eta");
  fAntiLambdaPtvsEta->GetZaxis()->SetBinLabel(1,"All events");
  fAntiLambdaPtvsEta->GetZaxis()->SetBinLabel(2,"Triggered events");
  fAntiLambdaPtvsEta->GetZaxis()->SetBinLabel(3,"All ev wi Inv Mass cut");
  fAntiLambdaPtvsEta->GetZaxis()->SetBinLabel(4,"Trig ev wi Inv Mass cut");
  fOutput->Add(fAntiLambdaPtvsEta);

  fAntiLambdaPtvsRap =
    new TH3F("fAntiLambdaPtvsRap","#bar{#Lambda}: p_{T} vs y",nbins,pMin,pMax,30,-1.5,1.5,4,-0.5,3.5);
  fAntiLambdaPtvsRap->GetXaxis()->SetTitle("p_{T} (GeV/c)");
  fAntiLambdaPtvsRap->GetYaxis()->SetTitle("y");
  fAntiLambdaPtvsRap->GetYaxis()->SetTitle("#eta");
  fAntiLambdaPtvsRap->GetZaxis()->SetBinLabel(1,"All events");
  fAntiLambdaPtvsRap->GetZaxis()->SetBinLabel(2,"Triggered events");
  fAntiLambdaPtvsRap->GetZaxis()->SetBinLabel(3,"All ev wi Inv Mass cut");
  fAntiLambdaPtvsRap->GetZaxis()->SetBinLabel(4,"Trig ev wi Inv Mass cut");
  fOutput->Add(fAntiLambdaPtvsRap);

  fAntiLambdaEtaPhi =
    new TH2F("fAntiLambdaEtaPhi","#phi vs #eta for #bar{#Lambda}",
	     nbinsPhi,0.,2.*TMath::Pi(),100,-1.,1.);
  fAntiLambdaEtaPhi->GetXaxis()->SetTitle("#phi (rad)"); 
  fAntiLambdaEtaPhi->GetYaxis()->SetTitle("#eta"); 
  fOutput->Add(fAntiLambdaEtaPhi);


 fAntiLambdaMassPtPhi  = 
    new TH3F("fAntiLambdaMassPtPhi","#bar{#Lambda}: mass vs pt vs #phi",
	     nbins,1.065,1.165,nbins,pMin,pMax,nbinsPhi,0.,2.*TMath::Pi());
  fAntiLambdaMassPtPhi->GetXaxis()->SetTitle("Mass (GeV/c^2)"); 
  fAntiLambdaMassPtPhi->GetYaxis()->SetTitle("p_{T} (GeV/c)"); 
  fAntiLambdaMassPtPhi->GetZaxis()->SetTitle("#phi (rad)"); 
  fOutput->Add(fAntiLambdaMassPtPhi);


  // Correlations:
  fAntiLambdaMassPtvsPtL = 
    new TH3F("fAntiLambdaMassPtvsPtL", "#bar{#Lambda}: mass vs p_{T} vs p_{T,l}",
	     nbins,1.065,1.165,
	     nbins,pMin,pMax,
	     nbinPtLP,pMin,ptMaxLP);
  fAntiLambdaMassPtvsPtL->GetXaxis()->SetTitle("Mass (GeV/c^2)"); 
  fAntiLambdaMassPtvsPtL->GetYaxis()->SetTitle("p_{T} (GeV/c)"); 
  fAntiLambdaMassPtvsPtL->GetZaxis()->SetTitle("p_{T,l} (GeV/c)"); 
  fOutput->Add(fAntiLambdaMassPtvsPtL);

  /*
  fAntiLambdaSiPtL =
    new TH3F("fAntiLambdaSiPtL","#bar{#Lambda}: L_{T} vs p_{T} vs p_{t,L}, side-band subtracted",
	     nbins,pMin,pMax,2*nbins,lMin,lMax,
	     nbinPtLP,pMin,ptMaxLP);
  fAntiLambdaSiPtL->GetXaxis()->SetTitle("p_{T} (GeV/c)"); 
  fAntiLambdaSiPtL->GetYaxis()->SetTitle("L_{T} (cm)"); 
  fAntiLambdaSiPtL->GetZaxis()->SetTitle("p_{T,l} (GeV/c)"); 
  fOutput->Add(fAntiLambdaSiPtL);

  fAntiLambdaDaughtersPt =
    new TH2F("fAntiLambdaDaughtersPt","#bar{#Lambda}: p_{T} of daughters",
	     nbins/2,pMin,pMax/2.,nbins/2,pMin,pMax/2.);
  fAntiLambdaDaughtersPt->GetXaxis()->SetTitle("Neg. Daught p_{T} (GeV/c)"); 
  fAntiLambdaDaughtersPt->GetYaxis()->SetTitle("Pos. Daught p_{T} (GeV/c)"); 
  fOutput->Add(fAntiLambdaDaughtersPt);
  */

  fAntiLambdadPhiPtAssocPtL =
    new TH3F("fAntiLambdadPhiPtAssocPtL","#bar{#Lambda}: #Delta#phi vs  p_{T,a} vs p_{T,l}",
	     nbinsPhi,-TMath::PiOver2(),3*TMath::PiOver2(),nbins,pMin,pMax,nbinPtLP,pMin,ptMaxLP);
  fAntiLambdadPhiPtAssocPtL->GetXaxis()->SetTitle("#Delta#phi (rad)"); 
  fAntiLambdadPhiPtAssocPtL->GetYaxis()->SetTitle("p_{T,a} (GeV/c)"); 
  fAntiLambdadPhiPtAssocPtL->GetZaxis()->SetTitle("p_{T,l} (GeV/c)"); 
  fOutput->Add(fAntiLambdadPhiPtAssocPtL);

  fAntiLambdaDCADaugToPrimVtx  
    = new TH3F("fAntiLambdaDCADaugToPrimVtx","#bar{#Lambda} Bckg: dca daughter vs. p_{T,l}",
	       90,0.,3.3,90,0.,3.3,nbinPtLP,pMin,ptMaxLP);
  fAntiLambdaDCADaugToPrimVtx->GetXaxis()->SetTitle("DCA Pos daug (cm)"); 
  fAntiLambdaDCADaugToPrimVtx->GetYaxis()->SetTitle("DCA Neg daug (cm)"); 
  fAntiLambdaDCADaugToPrimVtx->GetZaxis()->SetTitle("p_{T,l} (GeV/c)"); 
  fOutput->Add(fAntiLambdaDCADaugToPrimVtx);


  for(Int_t k=0;k<kN1;k++){

    // Monte-Carlo level:
    if(fIsMC){
    snprintf(hNameHist,100, "fAntiLambdadPhidEtaMC_%.1f_%.1f",kPtBinV0[k],kPtBinV0[k+1]); 
    fAntiLambdadPhidEtaMC[k] = new TH3F(hNameHist,"#bar{#Lambda} MC: #Delta#phi vs #Delta#eta vs p_{T,l}",
				  nbinsdPhi,-TMath::PiOver2(),3*TMath::PiOver2(),
				  nbinsdEta,-2.,2.,
				  nbinsVtx,-10.,10.);
    fAntiLambdadPhidEtaMC[k]->GetXaxis()->SetTitle("#Delta#phi (rad)"); 
    fAntiLambdadPhidEtaMC[k]->GetYaxis()->SetTitle("#Delta#eta"); 
    fAntiLambdadPhidEtaMC[k]->GetZaxis()->SetTitle("Vertex Z (cm)"); 
    fOutput->Add(fAntiLambdadPhidEtaMC[k]);
  
    snprintf(hNameHist,100, "fAntiLambdadPhidEtaMC_Cent_%.1f_%.1f",kPtBinV0[k],kPtBinV0[k+1]); 
    fAntiLambdadPhidEtaMCCent[k] = new TH3F(hNameHist,"#bar{#Lambda} MC: #Delta#phi vs #Delta#eta vs p_{T,l}",
				      nbinsdPhi,-TMath::PiOver2(),3*TMath::PiOver2(),
				      nbinsdEta,-2.,2.,
				      nbinsVtx,-10.,10.);
    fAntiLambdadPhidEtaMCCent[k]->GetXaxis()->SetTitle("#Delta#phi (rad)"); 
    fAntiLambdadPhidEtaMCCent[k]->GetYaxis()->SetTitle("#Delta#eta"); 
    fAntiLambdadPhidEtaMCCent[k]->GetZaxis()->SetTitle("Vertex Z (cm)");
    fOutput->Add(fAntiLambdadPhidEtaMCCent[k]);
    }

    // Reconstruction level:
    snprintf(hNameHist,100, "fAntiLambdadPhidEtaPtL_%.1f_%.1f",kPtBinV0[k],kPtBinV0[k+1]); 
    fAntiLambdadPhidEtaPtL[k] = new TH3F(hNameHist,"#bar{#Lambda}: #Delta#phi vs #Delta#eta vs p_{T,l}",
				     nbinsdPhi,-TMath::PiOver2(),3*TMath::PiOver2(),
				     nbinsdEta,-2.,2.,
				     nbinsVtx,-10.,10.);
    fAntiLambdadPhidEtaPtL[k]->GetXaxis()->SetTitle("#Delta#phi (rad)"); 
    fAntiLambdadPhidEtaPtL[k]->GetYaxis()->SetTitle("#Delta#eta"); 
    fAntiLambdadPhidEtaPtL[k]->GetZaxis()->SetTitle("Vertex Z (cm)");
    fOutput->Add(fAntiLambdadPhidEtaPtL[k]);
  
    snprintf(hNameHist,100, "fAntiLambdadPhidEtaPtL_Cent_%.1f_%.1f",kPtBinV0[k],kPtBinV0[k+1]); 
    fAntiLambdadPhidEtaPtLCent[k] = new TH3F(hNameHist,"#bar{#Lambda}: #Delta#phi  vs #Delta#eta vs p_{T,l}",
					 nbinsdPhi,-TMath::PiOver2(),3*TMath::PiOver2(),
					 nbinsdEta,-2.,2.,
					 nbinsVtx,-10.,10.);
    fAntiLambdadPhidEtaPtLCent[k]->GetXaxis()->SetTitle("#Delta#phi (rad)"); 
    fAntiLambdadPhidEtaPtLCent[k]->GetYaxis()->SetTitle("#Delta#eta"); 
    fAntiLambdadPhidEtaPtLCent[k]->GetZaxis()->SetTitle("Vertex Z (cm)");
    fOutput->Add(fAntiLambdadPhidEtaPtLCent[k]);

    snprintf(hNameHist,100, "fAntiLambdadPhidEtaPtL_Bckg_%.1f_%.1f",kPtBinV0[k],kPtBinV0[k+1]); 
    fAntiLambdadPhidEtaPtLBckg[k] = new TH3F(hNameHist,"#bar{#Lambda}: #Delta#phi  vs #Delta#eta vs p_{T,l}",
					 nbinsdPhi,-TMath::PiOver2(),3*TMath::PiOver2(),
					 nbinsdEta,-2.,2.,
					 nbinsVtx,-10.,10.);
    fAntiLambdadPhidEtaPtLBckg[k]->GetXaxis()->SetTitle("#Delta#phi (rad)"); 
    fAntiLambdadPhidEtaPtLBckg[k]->GetYaxis()->SetTitle("#Delta#eta"); 
    fAntiLambdadPhidEtaPtLBckg[k]->GetZaxis()->SetTitle("Vertex Z (cm)");
    fOutput->Add(fAntiLambdadPhidEtaPtLBckg[k]);

    snprintf(hNameHist,100, "fAntiLambdadPhidEtaPtL_Cent_Bckg_%.1f_%.1f",kPtBinV0[k],kPtBinV0[k+1]); 
    fAntiLambdadPhidEtaPtLCentBckg[k] = new TH3F(hNameHist,"#bar{#Lambda}: #Delta#phi  vs #Delta#eta vs p_{T,l}",
					     nbinsdPhi,-TMath::PiOver2(),3*TMath::PiOver2(),
					     nbinsdEta,-2.,2.,
					     nbinsVtx,-10.,10.);
    fAntiLambdadPhidEtaPtLCentBckg[k]->GetXaxis()->SetTitle("#Delta#phi (rad)"); 
    fAntiLambdadPhidEtaPtLCentBckg[k]->GetYaxis()->SetTitle("#Delta#eta"); 
    fAntiLambdadPhidEtaPtLCentBckg[k]->GetZaxis()->SetTitle("Vertex Z (cm)");
    fOutput->Add(fAntiLambdadPhidEtaPtLCentBckg[k]);


    //  To be used when the Armenteros Podolanski plot is applied
    snprintf(hNameHist,100, "fAntiLambdadPhidEtaPtL2_%.1f_%.1f",kPtBinV0[k],kPtBinV0[k+1]); 
    fAntiLambdadPhidEtaPtL2[k] = new TH3F(hNameHist,"#bar{#Lambda}: #Delta#phi vs #Delta#eta vs p_{T,l}",
				      nbinsdPhi,-TMath::PiOver2(),3*TMath::PiOver2(),
				      nbinsdEta,-2.,2.,
				      nbinsVtx,-10.,10.);
    fAntiLambdadPhidEtaPtL2[k]->GetXaxis()->SetTitle("#Delta#phi (rad)"); 
    fAntiLambdadPhidEtaPtL2[k]->GetYaxis()->SetTitle("#Delta#eta"); 
    fAntiLambdadPhidEtaPtL2[k]->GetZaxis()->SetTitle("Vertex Z (cm)");
    fOutput->Add(fAntiLambdadPhidEtaPtL2[k]);
  
    snprintf(hNameHist,100, "fAntiLambdadPhidEtaPtL2_Cent_%.1f_%.1f",kPtBinV0[k],kPtBinV0[k+1]); 
    fAntiLambdadPhidEtaPtLCent2[k] = new TH3F(hNameHist,"#bar{#Lambda}: #Delta#phi  vs #Delta#eta vs p_{T,l}",
					  nbinsdPhi,-TMath::PiOver2(),3*TMath::PiOver2(),
					  nbinsdEta,-2.,2.,
					  nbinsVtx,-10.,10.);
    fAntiLambdadPhidEtaPtLCent2[k]->GetXaxis()->SetTitle("#Delta#phi (rad)"); 
    fAntiLambdadPhidEtaPtLCent2[k]->GetYaxis()->SetTitle("#Delta#eta"); 
    fAntiLambdadPhidEtaPtLCent2[k]->GetZaxis()->SetTitle("Vertex Z (cm)");
    fOutput->Add(fAntiLambdadPhidEtaPtLCent2[k]);

    snprintf(hNameHist,100, "fAntiLambdadPhidEtaPtL2_Bckg_%.1f_%.1f",kPtBinV0[k],kPtBinV0[k+1]); 
    fAntiLambdadPhidEtaPtLBckg2[k] = new TH3F(hNameHist,"#bar{#Lambda}: #Delta#phi  vs #Delta#eta vs p_{T,l}",
					  nbinsdPhi,-TMath::PiOver2(),3*TMath::PiOver2(),
					  nbinsdEta,-2.,2.,
					  nbinsVtx,-10.,10.);
    fAntiLambdadPhidEtaPtLBckg2[k]->GetXaxis()->SetTitle("#Delta#phi (rad)"); 
    fAntiLambdadPhidEtaPtLBckg2[k]->GetYaxis()->SetTitle("#Delta#eta"); 
    fAntiLambdadPhidEtaPtLBckg2[k]->GetZaxis()->SetTitle("Vertex Z (cm)");
    fOutput->Add(fAntiLambdadPhidEtaPtLBckg2[k]);

    snprintf(hNameHist,100, "fAntiLambdadPhidEtaPtL2_Cent_Bckg_%.1f_%.1f",kPtBinV0[k],kPtBinV0[k+1]); 
    fAntiLambdadPhidEtaPtLCentBckg2[k] = new TH3F(hNameHist,"#bar{#Lambda}: #Delta#phi  vs #Delta#eta vs p_{T,l}",
					      nbinsdPhi,-TMath::PiOver2(),3*TMath::PiOver2(),
					      nbinsdEta,-2.,2.,
					      nbinsVtx,-10.,10.);
    fAntiLambdadPhidEtaPtLCentBckg2[k]->GetXaxis()->SetTitle("#Delta#phi (rad)"); 
    fAntiLambdadPhidEtaPtLCentBckg2[k]->GetYaxis()->SetTitle("#Delta#eta"); 
    fAntiLambdadPhidEtaPtLCentBckg2[k]->GetZaxis()->SetTitle("Vertex Z (cm)");
    fOutput->Add(fAntiLambdadPhidEtaPtLCentBckg2[k]);

  }
  

  // Correlations (side-band):
  fAntiLambdaBckgDecLength
    = new TH2F("fAntiLambdaBckgDecLength","#bar{#Lambda} Bckg: c#tau vs. p_{T,l}",
	       100,0.,25.,nbinPtLP,pMin,ptMaxLP);
  fAntiLambdaBckgDecLength->GetXaxis()->SetTitle("c#tau (cm)"); 
  fAntiLambdaBckgDecLength->GetYaxis()->SetTitle("p_{T,l} (GeV/c)"); 
  fOutput->Add(fAntiLambdaBckgDecLength);
  
  fAntiLambdaBckgDCADaugToPrimVtx  
    = new TH3F("fAntiLambdaBckgDCADaugToPrimVtx","#bar{#Lambda} Bckg: dca daughter vs. p_{T,l}",
	       90,0.,3.3,90,0.,3.3,nbinPtLP,pMin,ptMaxLP);
  fAntiLambdaBckgDCADaugToPrimVtx->GetXaxis()->SetTitle("DCA Pos daug (cm)"); 
  fAntiLambdaBckgDCADaugToPrimVtx->GetYaxis()->SetTitle("DCA Neg daug (cm)"); 
  fAntiLambdaBckgDCADaugToPrimVtx->GetZaxis()->SetTitle("p_{T,l} (GeV/c)"); 
  fOutput->Add(fAntiLambdaBckgDCADaugToPrimVtx);
  
  fAntiLambdaBckgEtaPhi = 
    new TH2F("fAntiLambdaBckgEtaPhi","#bar{#Lambda} Bckg: #phi vs #eta",
	     nbinsPhi,0.,2.*TMath::Pi(),100,-1.,1.);
  fAntiLambdaBckgEtaPhi->GetXaxis()->SetTitle("#phi (rad)"); 
  fAntiLambdaBckgEtaPhi->GetYaxis()->SetTitle("#eta"); 
  fOutput->Add(fAntiLambdaBckgEtaPhi);
    
  fAntiLambdaBckgPhiRadio
    = new TH2F("fAntiLambdaBckgPhiRadio","#bar{#Lambda} Bckg: #phi vs l_{T}",
	       nbinsPhi,0.,2.*TMath::Pi(),2*nbins,lMin,lMax);
  fAntiLambdaBckgPhiRadio->GetXaxis()->SetTitle("#phi (rad)"); 
  fAntiLambdaBckgPhiRadio->GetYaxis()->SetTitle("l_{T} (cm)"); 
  fOutput->Add(fAntiLambdaBckgPhiRadio);


  fAntiLambdaBckgDCANegDaugToPrimVtx  
    = new TH2F("fAntiLambdaBckgDCANegDaugToPrimVtx","#bar{#Lambda} Bckg: dca NegDaughter",
	       6,-0.5,5.5,90,0.,3.3);
  fAntiLambdaBckgDCANegDaugToPrimVtx->GetXaxis()->SetTitle("MC Production"); 
  fAntiLambdaBckgDCANegDaugToPrimVtx->GetXaxis()->SetBinLabel(1,"Rec");
  fAntiLambdaBckgDCANegDaugToPrimVtx->GetXaxis()->SetBinLabel(2,"Primary");
  fAntiLambdaBckgDCANegDaugToPrimVtx->GetXaxis()->SetBinLabel(3,"Weak Decay");
  fAntiLambdaBckgDCANegDaugToPrimVtx->GetXaxis()->SetBinLabel(4,"Gamma conv.");
  fAntiLambdaBckgDCANegDaugToPrimVtx->GetXaxis()->SetBinLabel(5,"Unidentified mother");
  fAntiLambdaBckgDCANegDaugToPrimVtx->GetXaxis()->SetBinLabel(6,"Other");
  fAntiLambdaBckgDCANegDaugToPrimVtx->GetYaxis()->SetTitle("DCA Neg Daug (cm)"); 
  fOutput->Add(fAntiLambdaBckgDCANegDaugToPrimVtx);


  fAntiLambdaBckgDCAPosDaugToPrimVtx  
    = new TH2F("fAntiLambdaBckgDCAPosDaugToPrimVtx","#bar{#Lambda} Bckg: dca PosDaughter",
	       6,-0.5,5.5,90,0.,3.3);
  fAntiLambdaBckgDCAPosDaugToPrimVtx->GetXaxis()->SetTitle("MC Production"); 
  fAntiLambdaBckgDCAPosDaugToPrimVtx->GetXaxis()->SetBinLabel(1,"Rec");
  fAntiLambdaBckgDCAPosDaugToPrimVtx->GetXaxis()->SetBinLabel(2,"Primary");
  fAntiLambdaBckgDCAPosDaugToPrimVtx->GetXaxis()->SetBinLabel(3,"Weak Decay");
  fAntiLambdaBckgDCAPosDaugToPrimVtx->GetXaxis()->SetBinLabel(4,"Gamma conv.");
  fAntiLambdaBckgDCAPosDaugToPrimVtx->GetXaxis()->SetBinLabel(5,"Unidentified mother");
  fAntiLambdaBckgDCAPosDaugToPrimVtx->GetXaxis()->SetBinLabel(6,"Other");
  fAntiLambdaBckgDCAPosDaugToPrimVtx->GetYaxis()->SetTitle("DCA Pos Daug (cm)"); 
  fOutput->Add(fAntiLambdaBckgDCAPosDaugToPrimVtx);

  fAntiLambdaMassCascade
    = new TH2F("fAntiLambdaMassCascade","Cascade Reconstruction in #bar{#Lambda} Bckg",650, 1.2, 2.5,4,0.5,4.5);
  fAntiLambdaMassCascade->GetXaxis()->SetTitle("Invariant Mass (GeV/c^{2})");
  fAntiLambdaMassCascade->GetYaxis()->SetTitle("Cascade type");
  fOutput->Add(fAntiLambdaMassCascade);

  // ============================================================= //

  if(fDoQA){

    // Quality Assurance K0s:
    // Track PID :
    //    --- signal ---
    fK0sPIDPosDaug  = 
      new TH3F("fK0sPIDPosDaug","K^{0}_{S}: dE/dx Pos. Daug.",50,0.2,3,50,0.,6.,nbins,pMin,pMax);
    fK0sPIDPosDaug->GetXaxis()->SetTitle("TPC Momentum (GeV/c)"); 
    fK0sPIDPosDaug->GetYaxis()->SetTitle("a.u."); 
    fK0sPIDPosDaug->GetZaxis()->SetTitle("p_{T} V0"); 
    fOutputQA->Add(fK0sPIDPosDaug);

    fK0sPIDNegDaug  = 
      new TH3F("fK0sPIDNegDaug","K^{0}_{S}: dE/dx Neg. Daug.",50,0.2,3,50,0.,6.,nbins,pMin,pMax);
    fK0sPIDNegDaug->GetXaxis()->SetTitle("TPC Momentum (GeV/c)"); 
    fK0sPIDNegDaug->GetYaxis()->SetTitle("a.u."); 
    fK0sPIDNegDaug->GetZaxis()->SetTitle("p_{T} V0"); 
    fOutputQA->Add(fK0sPIDNegDaug);

    //    --- background ---
    fK0sBckgPIDPosDaug  = 
      new TH3F("fK0sBckgPIDPosDaug","K^{0}_{S} Bckg: dE/dx Pos. Daug.",50,0.2,3,50,0.,6.,nbins,pMin,pMax);
    fK0sBckgPIDPosDaug->GetXaxis()->SetTitle("TPC Momentum (GeV/c)"); 
    fK0sBckgPIDPosDaug->GetYaxis()->SetTitle("a.u."); 
    fK0sBckgPIDPosDaug->GetZaxis()->SetTitle("p_{T} V0"); 
    fOutputQA->Add(fK0sBckgPIDPosDaug);

    fK0sBckgPIDNegDaug  = 
      new TH3F("fK0sBckgPIDNegDaug","K^{0}_{S} Bckg: dE/dx Neg. Daug.",50,0.2,3,50,0.,6.,nbins,pMin,pMax);
    fK0sBckgPIDNegDaug->GetXaxis()->SetTitle("TPC Momentum (GeV/c)"); 
    fK0sBckgPIDNegDaug->GetYaxis()->SetTitle("a.u."); 
    fK0sBckgPIDNegDaug->GetZaxis()->SetTitle("p_{T} V0"); 
    fOutputQA->Add(fK0sBckgPIDNegDaug);

    // Phi Eta
    //     --- signal ---
    fK0sPhiEtaPosDaug = 
      new TH3F("fK0sPhiEtaPosDaug","K^{0}_{S}: #phi vs #eta Pos. Daug.",nbinsPhi,0.,2.*TMath::Pi(),100,-1.,1.,nbins,pMin,pMax);
    fK0sPhiEtaPosDaug->GetXaxis()->SetTitle("#phi"); 
    fK0sPhiEtaPosDaug->GetYaxis()->SetTitle("#eta"); 
    fK0sPhiEtaPosDaug->GetZaxis()->SetTitle("p_{T} V0"); 
    fOutputQA->Add(fK0sPhiEtaPosDaug);

    fK0sPhiEtaNegDaug  = 
      new TH3F("fK0sPhiEtaNegDaug","K^{0}_{S}: #phi vs #eta Neg. Daug.",nbinsPhi,0.,2.*TMath::Pi(),100,-1.,1.,nbins,pMin,pMax);
    fK0sPhiEtaNegDaug->GetXaxis()->SetTitle("#phi"); 
    fK0sPhiEtaNegDaug->GetYaxis()->SetTitle("#eta"); 
    fK0sPhiEtaNegDaug->GetZaxis()->SetTitle("p_{T} V0"); 
    fOutputQA->Add(fK0sPhiEtaNegDaug);

    //     --- background ---
    fK0sBckgPhiEtaPosDaug = 
      new TH3F("fK0sBckgPhiEtaPosDaug","K^{0}_{S} Bckg: #phi vs #eta Pos. Daug.",nbinsPhi,0.,2.*TMath::Pi(),100,-1.,1.,nbins,pMin,pMax);
    fK0sBckgPhiEtaPosDaug->GetXaxis()->SetTitle("#phi"); 
    fK0sBckgPhiEtaPosDaug->GetYaxis()->SetTitle("#eta"); 
    fK0sBckgPhiEtaPosDaug->GetZaxis()->SetTitle("p_{T} V0"); 
    fOutputQA->Add(fK0sBckgPhiEtaPosDaug);

    fK0sBckgPhiEtaNegDaug  = 
      new TH3F("fK0sBckgPhiEtaNegDaug","K^{0}_{S} Bckg: #phi vs #eta Neg. Daug.",nbinsPhi,0.,2.*TMath::Pi(),100,-1.,1.,nbins,pMin,pMax);
    fK0sBckgPhiEtaNegDaug->GetXaxis()->SetTitle("#phi"); 
    fK0sBckgPhiEtaNegDaug->GetYaxis()->SetTitle("#eta"); 
    fK0sBckgPhiEtaNegDaug->GetZaxis()->SetTitle("p_{T} V0"); 
    fOutputQA->Add(fK0sBckgPhiEtaNegDaug);

    // Distance of closest approach:
    //     --- signal ---
    fK0sDCAPosDaug = 
      new TH2F("fK0sDCAPosDaug","K^{0}_{S}: dca Pos",90,0.,3.3,nbins,pMin,pMax);
    fK0sDCAPosDaug->GetXaxis()->SetTitle("dca"); 
    fK0sDCAPosDaug->GetYaxis()->SetTitle("p_{T} V0"); 
    fOutputQA->Add(fK0sDCAPosDaug);

    fK0sDCANegDaug =  
      new TH2F("fK0sDCANegDaug","K^{0}_{S}: dca Neg",90,0.,3.3,nbins,pMin,pMax);
    fK0sDCANegDaug->GetXaxis()->SetTitle("dca"); 
    fK0sDCANegDaug->GetYaxis()->SetTitle("p_{T} V0"); 
    fOutputQA->Add(fK0sDCANegDaug);
    
    //     --- background ---
    fK0sBckgDCAPosDaug = 
      new TH2F("fK0sBckgDCAPosDaug","K^{0}_{S} Bckg: dca Pos",90,0.,3.3,nbins,pMin,pMax);
    fK0sBckgDCAPosDaug->GetXaxis()->SetTitle("dca"); 
    fK0sBckgDCAPosDaug->GetYaxis()->SetTitle("p_{T} V0"); 
    fOutputQA->Add(fK0sBckgDCAPosDaug);

    fK0sBckgDCANegDaug =  
      new TH2F("fK0sBckgDCANegDaug","K^{0}_{S} Bckg: dca Neg",90,0.,3.3,nbins,pMin,pMax);
    fK0sBckgDCANegDaug->GetXaxis()->SetTitle("dca"); 
    fK0sBckgDCANegDaug->GetYaxis()->SetTitle("p_{T} V0"); 
    fOutputQA->Add(fK0sBckgDCANegDaug);

    // Difference in momentum:
    //     --- signal ---
    fK0sDifPtPosDaug =  
      new TH2F("fK0sDifPtPosDaug","K^{0}_{S}: dif. p_{T}",90,0.,3.3,nbins,pMin,pMax);
    fK0sDifPtPosDaug->GetXaxis()->SetTitle("#Delta p_{T}"); 
    fK0sDifPtPosDaug->GetYaxis()->SetTitle("p_{T} V0"); 
    fOutputQA->Add(fK0sDifPtPosDaug);

    fK0sDifPtNegDaug =  
      new TH2F("fK0sDifPtNegDaug","K^{0}_{S}: dif. p_{T}",90,0.,3.3,nbins,pMin,pMax);
    fK0sDifPtNegDaug->GetXaxis()->SetTitle("#Delta p_{T}"); 
    fK0sDifPtNegDaug->GetYaxis()->SetTitle("p_{T} V0"); 
    fOutputQA->Add(fK0sDifPtNegDaug);

    //     --- background ---
    fK0sBckgDifPtPosDaug =  
      new TH2F("fK0sBckgDifPtPosDaug","K^{0}_{S} Bckg: dif. p_{T}",90,0.,3.3,nbins,pMin,pMax);
    fK0sBckgDifPtPosDaug->GetXaxis()->SetTitle("#Delta p_{T}"); 
    fK0sBckgDifPtPosDaug->GetYaxis()->SetTitle("p_{T} V0"); 
    fOutputQA->Add(fK0sBckgDifPtPosDaug);

    fK0sBckgDifPtNegDaug =  
      new TH2F("fK0sBckgDifPtNegDaug","K^{0}_{S} Bckg: dif. p_{T}",90,0.,3.3,nbins,pMin,pMax);
    fK0sBckgDifPtNegDaug->GetXaxis()->SetTitle("#Delta p_{T}"); 
    fK0sBckgDifPtNegDaug->GetYaxis()->SetTitle("p_{T} V0"); 
    fOutputQA->Add(fK0sBckgDifPtNegDaug);


    // Decay vertex reconstruction:
    //     --- signal ---
    fK0sDecayPos  =  
      new TH3F("fK0sDecayPos","K^{0}_{S}: Position of Dec. Vtx",200,-100.,100.,200,-100.,100.,nbins,pMin,pMax);
    fK0sDecayPos->GetXaxis()->SetTitle("Pos. X"); 
    fK0sDecayPos->GetYaxis()->SetTitle("Pos. Y"); 
    fK0sDecayPos->GetZaxis()->SetTitle("p_{T} V0"); 
    fOutputQA->Add(fK0sDecayPos);

    fK0sDecayVertex  =  
      new TH2F("fK0sDecayVertex","K^{0}_{S}: decay lenght",100,0.,100.,nbins,pMin,pMax);
    fK0sDecayVertex->GetXaxis()->SetTitle("l_{T}"); 
    fK0sDecayVertex->GetYaxis()->SetTitle("p_{T} V0"); 
    fOutputQA->Add(fK0sDecayVertex);

    fK0sDecayVertexZoom  =  
    new TH2F("fK0sDecayVertexZoom","K^{0}_{S}: decay lenght",20,0.,1.,nbins,pMin,pMax);
    fK0sDecayVertexZoom->GetXaxis()->SetTitle("l_{T}"); 
    fK0sDecayVertexZoom->GetYaxis()->SetTitle("p_{T} V0"); 
    fOutputQA->Add(fK0sDecayVertexZoom);

    //     --- background ---
    fK0sBckgDecayPos  =  
      new TH3F("fK0sBckgDecayPos","K^{0}_{S}: Position of Dec. Vtx",200,-100.,100.,200,-100.,100.,nbins,pMin,pMax);
    fK0sBckgDecayPos->GetXaxis()->SetTitle("Pos. X"); 
    fK0sBckgDecayPos->GetYaxis()->SetTitle("Pos. Y"); 
    fK0sBckgDecayPos->GetZaxis()->SetTitle("p_{T} V0"); 
    fOutputQA->Add(fK0sBckgDecayPos);

    fK0sBckgDecayVertex  =  
      new TH2F("fK0sBckgDecayVertex","K^{0}_{S} Bckg: decay vertex",100,0.,100.,nbins,pMin,pMax);
    fK0sBckgDecayVertex->GetXaxis()->SetTitle("l_{T}"); 
    fK0sBckgDecayVertex->GetYaxis()->SetTitle("p_{T} V0"); 
    fOutputQA->Add(fK0sBckgDecayVertex);

    fK0sBckgDecayVertexZoom  =  
      new TH2F("fK0sBckgDecayVertexZoom","K^{0}_{S} Bckg: decay lenght",20,0.,1.,nbins,pMin,pMax);
    fK0sBckgDecayVertexZoom->GetXaxis()->SetTitle("l_{T}"); 
    fK0sBckgDecayVertexZoom->GetYaxis()->SetTitle("p_{T} V0"); 
    fOutputQA->Add(fK0sBckgDecayVertexZoom);

    // Cosine of the Pointing Angle:
    //     --- signal ---
    fK0sCPA  =  
      new TH2F("fK0sCPA","K^{0}_{S}: cosine of the pointing angle",100,0.9,1.,nbins,pMin,pMax);
    fK0sCPA->GetXaxis()->SetTitle("cpa"); 
    fK0sCPA->GetYaxis()->SetTitle("p_{T} V0"); 
    fOutputQA->Add(fK0sCPA);
    //     --- background ---
    fK0sBckgCPA  =  
      new TH2F("fK0sBckgCPA","K^{0}_{S} Bckg: cosine of the pointing angle",100,0.9,1.,nbins,pMin,pMax);
    fK0sBckgCPA->GetXaxis()->SetTitle("cpa"); 
    fK0sBckgCPA->GetYaxis()->SetTitle("p_{T} V0"); 
    fOutputQA->Add(fK0sBckgCPA);

    // DCA between daughters:
    //     --- signal ---
    fK0sDCAV0Daug  =  
      new TH2F("fK0sDCAV0Daug","K^{0}_{S}: DCA daughters",60,0,1.2,nbins,pMin,pMax);
    fK0sDCAV0Daug->GetXaxis()->SetTitle("dca between daughters"); 
    fK0sDCAV0Daug->GetYaxis()->SetTitle("p_{T} V0"); 
    fOutputQA->Add(fK0sDCAV0Daug);
    //     --- background ---
    fK0sBckgDCAV0Daug  =  
      new TH2F("fK0sBckgDCAV0Daug","K^{0}_{S} Bckg: DCA daughters",60,0,1.2,nbins,pMin,pMax);
    fK0sBckgDCAV0Daug->GetXaxis()->SetTitle("dca between daughters"); 
    fK0sBckgDCAV0Daug->GetYaxis()->SetTitle("p_{T} V0"); 
    fOutputQA->Add(fK0sBckgDCAV0Daug);

    // Number of TPC clusters:
    //     --- signal ---
    fK0sNClustersTPC = 
      new TH3F("fK0sNClustersTPCPos","K^{0}_{S};#phi;Num. TPC Clusters",nbinsPhi,0.,2.*TMath::Pi(),201,-0.5,200.5,3,-0.5,2.5); 
    fK0sNClustersTPC->GetZaxis()->SetBinLabel(1,"Pos. Daug.");
    fK0sNClustersTPC->GetZaxis()->SetBinLabel(3,"Neg. Daug.");
    fOutputQA->Add(fK0sNClustersTPC);
    //     --- background ---
    fK0sBckgNClustersTPC = 
      new TH3F("fK0sBckgNClustersTPC","K^{0}_{S} Bckg;#phi;Num. TPC Clusters",nbinsPhi,0.,2.*TMath::Pi(),201,-0.5,200.5,3,-0.5,2.5); 
    fK0sBckgNClustersTPC->GetZaxis()->SetBinLabel(1,"Pos. Daug.");
    fK0sBckgNClustersTPC->GetZaxis()->SetBinLabel(3,"Neg. Daug.");
    fOutputQA->Add(fK0sBckgNClustersTPC);
 
    // Number of ITS clusters:
    //     --- signal ---
    fK0sNClustersITSPos = 
      new TH3F("fK0sNClustersITSPos","K^{0}_{S}: Pos. Daug;#phi;Num. ITS Clusters;p_{T} (GeV/c)",nbinsPhi,0.,2.*TMath::Pi(),7,-0.5,6.5,nbins,pMin,pMax); 
    fOutputQA->Add(fK0sNClustersITSPos);

    fK0sNClustersITSNeg = 
      new TH3F("fK0sNClustersITSNeg","K^{0}_{S}: Neg. Daug;#phi;Num. ITS Clusters;p_{T} (GeV/c)",nbinsPhi,0.,2.*TMath::Pi(),7,-0.5,6.5,nbins,pMin,pMax); 
    fOutputQA->Add(fK0sNClustersITSNeg);
    //     --- background ---
    fK0sBckgNClustersITSPos = 
      new TH3F("fK0sBckgNClustersITSPos","K^{0}_{S} Bckg: Pos. Daug;#phi;Num. ITS Clusters;;p_{T} (GeV/c)",nbinsPhi,0.,2.*TMath::Pi(),7,-0.5,6.5,nbins,pMin,pMax); 
    fOutputQA->Add(fK0sBckgNClustersITSPos);

    fK0sBckgNClustersITSNeg = 
      new TH3F("fK0sBckgNClustersITSNeg","K^{0}_{S} Bckg: Neg. Daug;#phi;Num. ITS Clusters;;p_{T} (GeV/c)",nbinsPhi,0.,2.*TMath::Pi(),7,-0.5,6.5,nbins,pMin,pMax); 
    fOutputQA->Add(fK0sBckgNClustersITSNeg);
  

    // Quality Assurance Lambda:
    // Track PID :
    //    --- signal ---
    fLambdaPIDPosDaug  = 
      new TH3F("fLambdaPIDPosDaug","#Lambda: dE/dx Pos. Daug.",50,0.2,3,50,0.,6.,nbins,pMin,pMax);
    fLambdaPIDPosDaug->GetXaxis()->SetTitle("TPC Momentum (GeV/c)"); 
    fLambdaPIDPosDaug->GetYaxis()->SetTitle("a.u."); 
    fLambdaPIDPosDaug->GetZaxis()->SetTitle("p_{T} V0"); 
    fOutputQA->Add(fLambdaPIDPosDaug);

    fLambdaPIDNegDaug  = 
      new TH3F("fLambdaPIDNegDaug","#Lambda: dE/dx Neg. Daug.",50,0.2,3,50,0.,6.,nbins,pMin,pMax);
    fLambdaPIDNegDaug->GetXaxis()->SetTitle("TPC Momentum (GeV/c)"); 
    fLambdaPIDNegDaug->GetYaxis()->SetTitle("a.u."); 
    fLambdaPIDNegDaug->GetZaxis()->SetTitle("p_{T} V0"); 
    fOutputQA->Add(fLambdaPIDNegDaug);

    //    --- background ---
    fLambdaBckgPIDPosDaug  = 
      new TH3F("fLambdaBckgPIDPosDaug","#Lambda: dE/dx Pos. Daug.",50,0.2,3,50,0.,6.,nbins,pMin,pMax);
    fLambdaBckgPIDPosDaug->GetXaxis()->SetTitle("TPC Momentum (GeV/c)"); 
    fLambdaBckgPIDPosDaug->GetYaxis()->SetTitle("a.u."); 
    fLambdaBckgPIDPosDaug->GetZaxis()->SetTitle("p_{T} V0"); 
    fOutputQA->Add(fLambdaBckgPIDPosDaug);

    fLambdaBckgPIDNegDaug  = 
      new TH3F("fLambdaBckgPIDNegDaug","#Lambda: dE/dx Neg. Daug.",50,0.2,3,50,0.,6.,nbins,pMin,pMax);
    fLambdaBckgPIDNegDaug->GetXaxis()->SetTitle("TPC Momentum (GeV/c)"); 
    fLambdaBckgPIDNegDaug->GetYaxis()->SetTitle("a.u."); 
    fLambdaBckgPIDNegDaug->GetZaxis()->SetTitle("p_{T} V0"); 
    fOutputQA->Add(fLambdaBckgPIDNegDaug);

    // Phi Eta
    //     --- signal ---
    fLambdaPhiEtaPosDaug = 
      new TH3F("fLambdaPhiEtaPosDaug","#Lambda: #phi vs #eta Pos. Daug.",nbinsPhi,0.,2.*TMath::Pi(),100,-1.,1.,nbins,pMin,pMax);
    fLambdaPhiEtaPosDaug->GetXaxis()->SetTitle("#phi"); 
    fLambdaPhiEtaPosDaug->GetYaxis()->SetTitle("#eta"); 
    fLambdaPhiEtaPosDaug->GetZaxis()->SetTitle("p_{T} V0"); 
    fOutputQA->Add(fLambdaPhiEtaPosDaug);

    fLambdaPhiEtaNegDaug  = 
      new TH3F("fLambdaPhiEtaNegDaug","#Lambda: #phi vs #eta Neg. Daug.",nbinsPhi,0.,2.*TMath::Pi(),100,-1.,1.,nbins,pMin,pMax);
    fLambdaPhiEtaNegDaug->GetXaxis()->SetTitle("#phi"); 
    fLambdaPhiEtaNegDaug->GetYaxis()->SetTitle("#eta"); 
    fLambdaPhiEtaNegDaug->GetZaxis()->SetTitle("p_{T} V0"); 
    fOutputQA->Add(fLambdaPhiEtaNegDaug);

    //     --- background ---
    fLambdaBckgPhiEtaPosDaug = 
      new TH3F("fLambdaBckgPhiEtaPosDaug","#Lambda: #phi vs #eta Pos. Daug.",nbinsPhi,0.,2.*TMath::Pi(),100,-1.,1.,nbins,pMin,pMax);
    fLambdaBckgPhiEtaPosDaug->GetXaxis()->SetTitle("#phi"); 
    fLambdaBckgPhiEtaPosDaug->GetYaxis()->SetTitle("#eta"); 
    fLambdaBckgPhiEtaPosDaug->GetZaxis()->SetTitle("p_{T} V0"); 
    fOutputQA->Add(fLambdaBckgPhiEtaPosDaug);

    fLambdaBckgPhiEtaNegDaug  = 
      new TH3F("fLambdaBckgPhiEtaNegDaug","#Lambda: #phi vs #eta Neg. Daug.",nbinsPhi,0.,2.*TMath::Pi(),100,-1.,1.,nbins,pMin,pMax);
    fLambdaBckgPhiEtaNegDaug->GetXaxis()->SetTitle("#phi"); 
    fLambdaBckgPhiEtaNegDaug->GetYaxis()->SetTitle("#eta"); 
    fLambdaBckgPhiEtaNegDaug->GetZaxis()->SetTitle("p_{T} V0"); 
    fOutputQA->Add(fLambdaBckgPhiEtaNegDaug);

    // Distance of closest approach
    //     --- signal ---
    fLambdaDCAPosDaug = 
      new TH2F("fLambdaDCAPosDaug","#Lambda: dca Pos",90,0.,3.3,nbins,pMin,pMax);
    fLambdaDCAPosDaug->GetXaxis()->SetTitle("dca"); 
    fLambdaDCAPosDaug->GetYaxis()->SetTitle("p_{T} V0"); 
    fOutputQA->Add(fLambdaDCAPosDaug);

    fLambdaDCANegDaug =  
      new TH2F("fLambdaDCANegDaug","#Lambda: dca Neg",90,0.,3.3,nbins,pMin,pMax);
    fLambdaDCANegDaug->GetXaxis()->SetTitle("dca"); 
    fLambdaDCANegDaug->GetYaxis()->SetTitle("p_{T} V0"); 
    fOutputQA->Add(fLambdaDCANegDaug);
    
    //     --- background ---
    fLambdaBckgDCAPosDaug = 
      new TH2F("fLambdaBckgDCAPosDaug","#Lambda Bckg: dca Pos",90,0.,3.3,nbins,pMin,pMax);
    fLambdaBckgDCAPosDaug->GetXaxis()->SetTitle("dca"); 
    fLambdaBckgDCAPosDaug->GetYaxis()->SetTitle("p_{T} V0"); 
    fOutputQA->Add(fLambdaBckgDCAPosDaug);

    fLambdaBckgDCANegDaug =  
      new TH2F("fLambdaBckgDCANegDaug","#Lambda Bckg: dca Neg",90,0.,3.3,nbins,pMin,pMax);
    fLambdaBckgDCANegDaug->GetXaxis()->SetTitle("dca"); 
    fLambdaBckgDCANegDaug->GetYaxis()->SetTitle("p_{T} V0"); 
    fOutputQA->Add(fLambdaBckgDCANegDaug);

    // Difference in momentum
    //     --- signal ---
    fLambdaDifPtPosDaug =  
      new TH2F("fLambdaDifPtPosDaug","#Lambda: dif. p_{T} Pos",90,0.,3.3,nbins,pMin,pMax);
    fLambdaDifPtPosDaug->GetXaxis()->SetTitle("#Delta p_{T}"); 
    fLambdaDifPtPosDaug->GetYaxis()->SetTitle("p_{T} V0"); 
    fOutputQA->Add(fLambdaDifPtPosDaug);

    fLambdaDifPtNegDaug =  
      new TH2F("fLambdaDifPtNegDaug","#Lambda: dif. p_{T} Neg",90,0.,3.3,nbins,pMin,pMax);
    fLambdaDifPtNegDaug->GetXaxis()->SetTitle("#Delta p_{T}"); 
    fLambdaDifPtNegDaug->GetYaxis()->SetTitle("p_{T} V0"); 
    fOutputQA->Add(fLambdaDifPtNegDaug);

    //     --- background ---
    fLambdaBckgDifPtPosDaug =  
      new TH2F("fLambdaBckgDifPtPosDaug","#Lambda Bckg: dif. p_{T} Pos.",90,0.,3.3,nbins,pMin,pMax);
    fLambdaBckgDifPtPosDaug->GetXaxis()->SetTitle("#Delta p_{T}"); 
    fLambdaBckgDifPtPosDaug->GetYaxis()->SetTitle("p_{T} V0"); 
    fOutputQA->Add(fLambdaBckgDifPtPosDaug);

    fLambdaBckgDifPtNegDaug =  
      new TH2F("fLambdaBckgDifPtNegDaug","#Lambda Bckg: dif. p_{t} Neg",90,0.,3.3,nbins,pMin,pMax);
    fLambdaBckgDifPtNegDaug->GetXaxis()->SetTitle("#Delta p_{T}"); 
    fLambdaBckgDifPtNegDaug->GetYaxis()->SetTitle("p_{T} V0"); 
    fOutputQA->Add(fLambdaBckgDifPtNegDaug);


    // Decay vertex reconstruction
    //     --- signal ---
    fLambdaDecayPos  =  
      new TH3F("fLambdaDecayPos","#Lambda: Position of Dec. Vtx",200,-100.,100.,200,-100.,100.,nbins,pMin,pMax);
    fLambdaDecayPos->GetXaxis()->SetTitle("Pos. X"); 
    fLambdaDecayPos->GetYaxis()->SetTitle("Pos. Y"); 
    fLambdaDecayPos->GetZaxis()->SetTitle("p_{T} V0"); 
    fOutputQA->Add(fLambdaDecayPos);

    fLambdaDecayVertex  =  
      new TH2F("fLambdaDecayVertex","#Lambda: decay lenght",100,0.,100.,nbins,pMin,pMax);
    fLambdaDecayVertex->GetXaxis()->SetTitle("l_{T}"); 
    fLambdaDecayVertex->GetYaxis()->SetTitle("p_{T} V0"); 
    fOutputQA->Add(fLambdaDecayVertex);

    fLambdaDecayVertexZoom  =  
      new TH2F("fLambdaDecayVertexZoom","#Lambda: decay lenght",20,0.,1.,nbins,pMin,pMax);
    fLambdaDecayVertexZoom->GetXaxis()->SetTitle("l_{T}"); 
    fLambdaDecayVertexZoom->GetYaxis()->SetTitle("p_{T} V0"); 
    fOutputQA->Add(fLambdaDecayVertexZoom);

    //     --- background ---
    fLambdaBckgDecayPos  =  
      new TH3F("fLambdaBckgDecayPos","#Lambda Bckg: Position of Dec. Vtx",200,-100.,100.,200,-100.,100.,nbins,pMin,pMax);
    fLambdaBckgDecayPos->GetXaxis()->SetTitle("Pos. X"); 
    fLambdaBckgDecayPos->GetYaxis()->SetTitle("Pos. Y"); 
    fLambdaBckgDecayPos->GetZaxis()->SetTitle("p_{T} V0"); 
    fOutputQA->Add(fLambdaBckgDecayPos);

    fLambdaBckgDecayVertex  =  
      new TH2F("fLambdaBckgDecayVertex","#Lambda Bckg: decay lenght",100,0.,100.,nbins,pMin,pMax);
    fLambdaBckgDecayVertex->GetXaxis()->SetTitle("l_{T}"); 
    fLambdaBckgDecayVertex->GetYaxis()->SetTitle("p_{T} V0"); 
    fOutputQA->Add(fLambdaBckgDecayVertex);

    fLambdaBckgDecayVertexZoom  =  
      new TH2F("fLambdaBckgDecayVertexZoom","#Lambda Bckg: decay lenght",20,0.,1.,nbins,pMin,pMax);
    fLambdaBckgDecayVertexZoom->GetXaxis()->SetTitle("l_{T}"); 
    fLambdaBckgDecayVertexZoom->GetYaxis()->SetTitle("p_{T} V0"); 
    fOutputQA->Add(fLambdaBckgDecayVertexZoom);

    // Cosine of the Pointing Angle
    //     --- signal ---
    fLambdaCPA  =  
      new TH2F("fLambdaCPA","#Lambda: cosine of the pointing angle",100,0.9,1.,nbins,pMin,pMax);
    fLambdaCPA->GetXaxis()->SetTitle("cpa"); 
    fLambdaCPA->GetYaxis()->SetTitle("p_{T} V0"); 
    fOutputQA->Add(fLambdaCPA);
    //     --- background ---
    fLambdaBckgCPA  =  
      new TH2F("fLambdaBckgCPA","#Lambda Bckg: cosine of the pointing angle",100,0.9,1.,nbins,pMin,pMax);
    fLambdaBckgCPA->GetXaxis()->SetTitle("cpa"); 
    fLambdaBckgCPA->GetYaxis()->SetTitle("p_{T} V0"); 
    fOutputQA->Add(fLambdaBckgCPA);

    // DCA between daughters
    //     --- signal ---
    fLambdaDCAV0Daug  =  
      new TH2F("fLambdaDCAV0Daug","#Lambda: DCA daughters",60,0,1.2,nbins,pMin,pMax);
    fLambdaDCAV0Daug->GetXaxis()->SetTitle("dca between daughters"); 
    fLambdaDCAV0Daug->GetYaxis()->SetTitle("p_{T} V0"); 
    fOutputQA->Add(fLambdaDCAV0Daug);
    //     --- background ---
    fLambdaBckgDCAV0Daug  =  
      new TH2F("fLambdaBckgDCAV0Daug","#Lambda Bckg: DCA daughters",60,0,1.2,nbins,pMin,pMax);
    fLambdaBckgDCAV0Daug->GetXaxis()->SetTitle("dca between daughters"); 
    fLambdaBckgDCAV0Daug->GetYaxis()->SetTitle("p_{T} V0"); 
    fOutputQA->Add(fLambdaBckgDCAV0Daug);
  
    // Number of TPC clusters:
    //     --- signal ---
    fLambdaNClustersTPC = 
      new TH3F("fLambdaNClustersTPCPos","#Lambda;#phi;Num. TPC Clusters",nbinsPhi,0.,2.*TMath::Pi(),201,-0.5,200.5,3,-0.5,2.5); 
    fLambdaNClustersTPC->GetZaxis()->SetBinLabel(1,"Pos. Daug.");
    fLambdaNClustersTPC->GetZaxis()->SetBinLabel(3,"Neg. Daug.");
    fOutputQA->Add(fLambdaNClustersTPC);
    //     --- background ---
    fLambdaBckgNClustersTPC = 
      new TH3F("fLambdaBckgNClustersTPC","#Lambda Bckg;#phi;Num. TPC Clusters",nbinsPhi,0.,2.*TMath::Pi(),201,-0.5,200.5,3,-0.5,2.5); 
    fLambdaBckgNClustersTPC->GetZaxis()->SetBinLabel(1,"Pos. Daug.");
    fLambdaBckgNClustersTPC->GetZaxis()->SetBinLabel(3,"Neg. Daug.");
    fOutputQA->Add(fLambdaBckgNClustersTPC);
 
    // Number of ITS clusters:
    //     --- signal ---
    fLambdaNClustersITSPos = 
      new TH3F("fLambdaNClustersITSPos","#Lambda: Pos. Daug;#phi;Num. ITS Clusters;p_{T} (GeV/c)",nbinsPhi,0.,2.*TMath::Pi(),7,-0.5,6.5,nbins,pMin,pMax); 
    fOutputQA->Add(fLambdaNClustersITSPos);

    fLambdaNClustersITSNeg = 
      new TH3F("fLambdaNClustersITSNeg","#Lambda: Neg. Daug;#phi;Num. ITS Clusters;p_{T} (GeV/c)",nbinsPhi,0.,2.*TMath::Pi(),7,-0.5,6.5,nbins,pMin,pMax); 
    fOutputQA->Add(fLambdaNClustersITSNeg);
    //     --- background ---
    fLambdaBckgNClustersITSPos = 
      new TH3F("fLambdaBckgNClustersITSPos","#Lambda Bckg: Pos. Daug;#phi;Num. ITS Clusters;;p_{T} (GeV/c)",nbinsPhi,0.,2.*TMath::Pi(),7,-0.5,6.5,nbins,pMin,pMax); 
    fOutputQA->Add(fLambdaBckgNClustersITSPos);

    fLambdaBckgNClustersITSNeg = 
      new TH3F("fLambdaBckgNClustersITSNeg","#Lambda Bckg: Neg. Daug;#phi;Num. ITS Clusters;;p_{T} (GeV/c)",nbinsPhi,0.,2.*TMath::Pi(),7,-0.5,6.5,nbins,pMin,pMax); 
    fOutputQA->Add(fLambdaBckgNClustersITSNeg);



    // Quality Assurance AntiLambda:
    // Track PID :
    //    --- signal ---
    fAntiLambdaPIDPosDaug  = 
      new TH3F("fAntiLambdaPIDPosDaug","#bar{#Lambda}: dE/dx Pos. Daug.",50,0.2,3,50,0.,6.,nbins,pMin,pMax);
    fAntiLambdaPIDPosDaug->GetXaxis()->SetTitle("TPC Momentum (GeV/c)"); 
    fAntiLambdaPIDPosDaug->GetYaxis()->SetTitle("a.u."); 
    fAntiLambdaPIDPosDaug->GetZaxis()->SetTitle("p_{T} V0"); 
    fOutputQA->Add(fAntiLambdaPIDPosDaug);

    fAntiLambdaPIDNegDaug  = 
      new TH3F("fAntiLambdaPIDNegDaug","#bar{#Lambda}: dE/dx Neg. Daug.",50,0.2,3,50,0.,6.,nbins,pMin,pMax);
    fAntiLambdaPIDNegDaug->GetXaxis()->SetTitle("TPC Momentum (GeV/c)"); 
    fAntiLambdaPIDNegDaug->GetYaxis()->SetTitle("a.u."); 
    fAntiLambdaPIDNegDaug->GetZaxis()->SetTitle("p_{T} V0"); 
    fOutputQA->Add(fAntiLambdaPIDNegDaug);

    //    --- background ---
    fAntiLambdaBckgPIDPosDaug  = 
      new TH3F("fAntiLambdaBckgPIDPosDaug","#bar{#Lambda}: dE/dx Pos. Daug.",50,0.2,3,50,0.,6.,nbins,pMin,pMax);
    fAntiLambdaBckgPIDPosDaug->GetXaxis()->SetTitle("TPC Momentum (GeV/c)"); 
    fAntiLambdaBckgPIDPosDaug->GetYaxis()->SetTitle("a.u."); 
    fAntiLambdaBckgPIDPosDaug->GetZaxis()->SetTitle("p_{T} V0"); 
    fOutputQA->Add(fAntiLambdaBckgPIDPosDaug);

    fAntiLambdaBckgPIDNegDaug  = 
      new TH3F("fAntiLambdaBckgPIDNegDaug","#bar{#Lambda}: dE/dx Neg. Daug.",50,0.2,3,50,0.,6.,nbins,pMin,pMax);
    fAntiLambdaBckgPIDNegDaug->GetXaxis()->SetTitle("TPC Momentum (GeV/c)"); 
    fAntiLambdaBckgPIDNegDaug->GetYaxis()->SetTitle("a.u."); 
    fAntiLambdaBckgPIDNegDaug->GetZaxis()->SetTitle("p_{T} V0"); 
    fOutputQA->Add(fAntiLambdaBckgPIDNegDaug);

    // Phi Eta
    //     --- signal ---
    fAntiLambdaPhiEtaPosDaug = 
      new TH3F("fAntiLambdaPhiEtaPosDaug","#bar{#Lambda}: #phi vs #eta Pos. Daug.",nbinsPhi,0.,2.*TMath::Pi(),100,-1.,1.,nbins,pMin,pMax);
    fAntiLambdaPhiEtaPosDaug->GetXaxis()->SetTitle("#phi"); 
    fAntiLambdaPhiEtaPosDaug->GetYaxis()->SetTitle("#eta"); 
    fAntiLambdaPhiEtaPosDaug->GetZaxis()->SetTitle("p_{T} V0"); 
    fOutputQA->Add(fAntiLambdaPhiEtaPosDaug);

    fAntiLambdaPhiEtaNegDaug  = 
      new TH3F("fAntiLambdaPhiEtaNegDaug","#bar{#Lambda}: #phi vs #eta Neg. Daug.",nbinsPhi,0.,2.*TMath::Pi(),100,-1.,1.,nbins,pMin,pMax);
    fAntiLambdaPhiEtaNegDaug->GetXaxis()->SetTitle("#phi"); 
    fAntiLambdaPhiEtaNegDaug->GetYaxis()->SetTitle("#eta"); 
    fAntiLambdaPhiEtaNegDaug->GetZaxis()->SetTitle("p_{T} V0"); 
    fOutputQA->Add(fAntiLambdaPhiEtaNegDaug);

    //     --- background ---
    fAntiLambdaBckgPhiEtaPosDaug = 
      new TH3F("fAntiLambdaBckgPhiEtaPosDaug","#bar{#Lambda}: #phi vs #eta Pos. Daug.",nbinsPhi,0.,2.*TMath::Pi(),100,-1.,1.,nbins,pMin,pMax);
    fAntiLambdaBckgPhiEtaPosDaug->GetXaxis()->SetTitle("#phi"); 
    fAntiLambdaBckgPhiEtaPosDaug->GetYaxis()->SetTitle("#eta"); 
    fAntiLambdaBckgPhiEtaPosDaug->GetZaxis()->SetTitle("p_{T} V0"); 
    fOutputQA->Add(fAntiLambdaBckgPhiEtaPosDaug);

    fAntiLambdaBckgPhiEtaNegDaug  = 
      new TH3F("fAntiLambdaBckgPhiEtaNegDaug","#bar{#Lambda}: #phi vs #eta Neg. Daug.",nbinsPhi,0.,2.*TMath::Pi(),100,-1.,1.,nbins,pMin,pMax);
    fAntiLambdaBckgPhiEtaNegDaug->GetXaxis()->SetTitle("#phi"); 
    fAntiLambdaBckgPhiEtaNegDaug->GetYaxis()->SetTitle("#eta"); 
    fAntiLambdaBckgPhiEtaNegDaug->GetZaxis()->SetTitle("p_{T} V0"); 
    fOutputQA->Add(fAntiLambdaBckgPhiEtaNegDaug);

    // Distance of closest approach
    //     --- signal ---
    fAntiLambdaDCAPosDaug = 
      new TH2F("fAntiLambdaDCAPosDaug","#bar{#Lambda}: dca Pos",90,0.,3.3,nbins,pMin,pMax);
    fAntiLambdaDCAPosDaug->GetXaxis()->SetTitle("dca"); 
    fAntiLambdaDCAPosDaug->GetYaxis()->SetTitle("p_{T} V0"); 
    fOutputQA->Add(fAntiLambdaDCAPosDaug);

    fAntiLambdaDCANegDaug =  
      new TH2F("fAntiLambdaDCANegDaug","#bar{#Lambda}: dca Neg",90,0.,3.3,nbins,pMin,pMax);
    fAntiLambdaDCANegDaug->GetXaxis()->SetTitle("dca"); 
    fAntiLambdaDCANegDaug->GetYaxis()->SetTitle("p_{T} V0"); 
    fOutputQA->Add(fAntiLambdaDCANegDaug);
    
    //     --- background ---
    fAntiLambdaBckgDCAPosDaug = 
      new TH2F("fAntiLambdaBckgDCAPosDaug","#bar{#Lambda} Bckg: dca Pos",90,0.,3.3,nbins,pMin,pMax);
    fAntiLambdaBckgDCAPosDaug->GetXaxis()->SetTitle("dca"); 
    fAntiLambdaBckgDCAPosDaug->GetYaxis()->SetTitle("p_{T} V0"); 
    fOutputQA->Add(fAntiLambdaBckgDCAPosDaug);

    fAntiLambdaBckgDCANegDaug =  
      new TH2F("fAntiLambdaBckgDCANegDaug","#bar{#Lambda} Bckg: dca Neg",90,0.,3.3,nbins,pMin,pMax);
    fAntiLambdaBckgDCANegDaug->GetXaxis()->SetTitle("dca"); 
    fAntiLambdaBckgDCANegDaug->GetYaxis()->SetTitle("p_{T} V0"); 
    fOutputQA->Add(fAntiLambdaBckgDCANegDaug);

    // Difference in momentum
    //     --- signal ---
    fAntiLambdaDifPtPosDaug =  
      new TH2F("fAntiLambdaDifPtPosDaug","#bar{#Lambda}: dif. p_{T} Pos",90,0.,3.3,nbins,pMin,pMax);
    fAntiLambdaDifPtPosDaug->GetXaxis()->SetTitle("#Delta p_{T}"); 
    fAntiLambdaDifPtPosDaug->GetYaxis()->SetTitle("p_{T} V0"); 
    fOutputQA->Add(fAntiLambdaDifPtPosDaug);

    fAntiLambdaDifPtNegDaug =  
      new TH2F("fAntiLambdaDifPtNegDaug","#bar{#Lambda}: dif. p_{T} Neg",90,0.,3.3,nbins,pMin,pMax);
    fAntiLambdaDifPtNegDaug->GetXaxis()->SetTitle("#Delta p_{T}"); 
    fAntiLambdaDifPtNegDaug->GetYaxis()->SetTitle("p_{T} V0"); 
    fOutputQA->Add(fAntiLambdaDifPtNegDaug);

    //     --- background ---
    fAntiLambdaBckgDifPtPosDaug =  
      new TH2F("fAntiLambdaBckgDifPtPosDaug","#bar{#Lambda} Bckg: dif. p_{T} Pos.",90,0.,3.3,nbins,pMin,pMax);
    fAntiLambdaBckgDifPtPosDaug->GetXaxis()->SetTitle("#Delta p_{T}"); 
    fAntiLambdaBckgDifPtPosDaug->GetYaxis()->SetTitle("p_{T} V0"); 
    fOutputQA->Add(fAntiLambdaBckgDifPtPosDaug);

    fAntiLambdaBckgDifPtNegDaug =  
      new TH2F("fAntiLambdaBckgDifPtNegDaug","#bar{#Lambda} Bckg: dif. p_{t} Neg",90,0.,3.3,nbins,pMin,pMax);
    fAntiLambdaBckgDifPtNegDaug->GetXaxis()->SetTitle("#Delta p_{T}"); 
    fAntiLambdaBckgDifPtNegDaug->GetYaxis()->SetTitle("p_{T} V0"); 
    fOutputQA->Add(fAntiLambdaBckgDifPtNegDaug);


    // Decay vertex reconstruction
    //     --- signal ---
    fAntiLambdaDecayPos  =  
      new TH3F("fAntiLambdaDecayPos","#bar{#Lambda}: Position of Dec. Vtx",200,-100.,100.,200,-100.,100.,nbins,pMin,pMax);
    fAntiLambdaDecayPos->GetXaxis()->SetTitle("Pos. X"); 
    fAntiLambdaDecayPos->GetYaxis()->SetTitle("Pos. Y"); 
    fAntiLambdaDecayPos->GetZaxis()->SetTitle("p_{T} V0"); 
    fOutputQA->Add(fAntiLambdaDecayPos);

    fAntiLambdaDecayVertex  =  
      new TH2F("fAntiLambdaDecayVertex","#bar{#Lambda}: decay lenght",100,0.,100.,nbins,pMin,pMax);
    fAntiLambdaDecayVertex->GetXaxis()->SetTitle("l_{T}"); 
    fAntiLambdaDecayVertex->GetYaxis()->SetTitle("p_{T} V0"); 
    fOutputQA->Add(fAntiLambdaDecayVertex);

    fAntiLambdaDecayVertexZoom  =  
      new TH2F("fAntiLambdaDecayVertexZoom","#bar{#Lambda}: decay lenght",20,0.,1.,nbins,pMin,pMax);
    fAntiLambdaDecayVertexZoom->GetXaxis()->SetTitle("l_{T}"); 
    fAntiLambdaDecayVertexZoom->GetYaxis()->SetTitle("p_{T} V0"); 
    fOutputQA->Add(fAntiLambdaDecayVertexZoom);

    //     --- background ---
    fAntiLambdaBckgDecayPos  =  
      new TH3F("fAntiLambdaBckgDecayPos","#bar{#Lambda} Bckg: Position of Dec. Vtx",200,-100.,100.,200,-100.,100.,nbins,pMin,pMax);
    fAntiLambdaBckgDecayPos->GetXaxis()->SetTitle("Pos. X"); 
    fAntiLambdaBckgDecayPos->GetYaxis()->SetTitle("Pos. Y"); 
    fAntiLambdaBckgDecayPos->GetZaxis()->SetTitle("p_{T} V0"); 
    fOutputQA->Add(fAntiLambdaBckgDecayPos);

    fAntiLambdaBckgDecayVertex  =  
      new TH2F("fAntiLambdaBckgDecayVertex","#bar{#Lambda} Bckg: decay lenght",100,0.,100.,nbins,pMin,pMax);
    fAntiLambdaBckgDecayVertex->GetXaxis()->SetTitle("l_{T}"); 
    fAntiLambdaBckgDecayVertex->GetYaxis()->SetTitle("p_{T} V0"); 
    fOutputQA->Add(fAntiLambdaBckgDecayVertex);

    fAntiLambdaBckgDecayVertexZoom  =  
      new TH2F("fAntiLambdaBckgDecayVertexZoom","#bar{#Lambda} Bckg: decay lenght",20,0.,1.,nbins,pMin,pMax);
    fAntiLambdaBckgDecayVertexZoom->GetXaxis()->SetTitle("l_{T}"); 
    fAntiLambdaBckgDecayVertexZoom->GetYaxis()->SetTitle("p_{T} V0"); 
    fOutputQA->Add(fAntiLambdaBckgDecayVertexZoom);

    // Cosine of the Pointing Angle
    //     --- signal ---
    fAntiLambdaCPA  =  
      new TH2F("fAntiLambdaCPA","#bar{#Lambda}: cosine of the pointing angle",100,0.9,1.,nbins,pMin,pMax);
    fAntiLambdaCPA->GetXaxis()->SetTitle("cpa"); 
    fAntiLambdaCPA->GetYaxis()->SetTitle("p_{T} V0"); 
    fOutputQA->Add(fAntiLambdaCPA);
    //     --- background ---
    fAntiLambdaBckgCPA  =  
      new TH2F("fAntiLambdaBckgCPA","#bar{#Lambda} Bckg: cosine of the pointing angle",100,0.9,1.,nbins,pMin,pMax);
    fAntiLambdaBckgCPA->GetXaxis()->SetTitle("cpa"); 
    fAntiLambdaBckgCPA->GetYaxis()->SetTitle("p_{T} V0"); 
    fOutputQA->Add(fAntiLambdaBckgCPA);

    // DCA between daughters
    //     --- signal ---
    fAntiLambdaDCAV0Daug  =  
      new TH2F("fAntiLambdaDCAV0Daug","#bar{#Lambda}: DCA daughters",60,0,1.2,nbins,pMin,pMax);
    fAntiLambdaDCAV0Daug->GetXaxis()->SetTitle("dca between daughters"); 
    fAntiLambdaDCAV0Daug->GetYaxis()->SetTitle("p_{T} V0"); 
    fOutputQA->Add(fAntiLambdaDCAV0Daug);
    //     --- background ---
    fAntiLambdaBckgDCAV0Daug  =  
      new TH2F("fAntiLambdaBckgDCAV0Daug","#bar{#Lambda} Bckg: DCA daughters",60,0,1.2,nbins,pMin,pMax);
    fAntiLambdaBckgDCAV0Daug->GetXaxis()->SetTitle("dca between daughters"); 
    fAntiLambdaBckgDCAV0Daug->GetYaxis()->SetTitle("p_{T} V0"); 
    fOutputQA->Add(fAntiLambdaBckgDCAV0Daug);

    // Number of TPC clusters:
    //     --- signal ---
    fAntiLambdaNClustersTPC = 
      new TH3F("fAntiLambdaNClustersTPCPos","#bar{#Lambda};#phi;Num. TPC Clusters",nbinsPhi,0.,2.*TMath::Pi(),201,-0.5,200.5,3,-0.5,2.5); 
    fAntiLambdaNClustersTPC->GetZaxis()->SetBinLabel(1,"Pos. Daug.");
    fAntiLambdaNClustersTPC->GetZaxis()->SetBinLabel(3,"Neg. Daug.");
    fOutputQA->Add(fAntiLambdaNClustersTPC);
    //     --- background ---
    fAntiLambdaBckgNClustersTPC = 
      new TH3F("fAntiLambdaBckgNClustersTPC","#bar{#Lambda} Bckg;#phi;Num. TPC Clusters",nbinsPhi,0.,2.*TMath::Pi(),201,-0.5,200.5,3,-0.5,2.5); 
    fAntiLambdaBckgNClustersTPC->GetZaxis()->SetBinLabel(1,"Pos. Daug.");
    fAntiLambdaBckgNClustersTPC->GetZaxis()->SetBinLabel(3,"Neg. Daug.");
    fOutputQA->Add(fAntiLambdaBckgNClustersTPC);
 
    // Number of ITS clusters:
    //     --- signal ---
    fAntiLambdaNClustersITSPos = 
      new TH3F("fAntiLambdaNClustersITSPos","#bar{#Lambda}: Pos. Daug;#phi;Num. ITS Clusters;p_{T} (GeV/c)",nbinsPhi,0.,2.*TMath::Pi(),7,-0.5,6.5,nbins,pMin,pMax); 
    fOutputQA->Add(fAntiLambdaNClustersITSPos);

    fAntiLambdaNClustersITSNeg = 
      new TH3F("fAntiLambdaNClustersITSNeg","#bar{#Lambda}: Neg. Daug;#phi;Num. ITS Clusters;p_{T} (GeV/c)",nbinsPhi,0.,2.*TMath::Pi(),7,-0.5,6.5,nbins,pMin,pMax); 
    fOutputQA->Add(fAntiLambdaNClustersITSNeg);
    //     --- background ---
    fAntiLambdaBckgNClustersITSPos = 
      new TH3F("fAntiLambdaBckgNClustersITSPos","#bar{#Lambda} Bckg: Pos. Daug;#phi;Num. ITS Clusters;;p_{T} (GeV/c)",nbinsPhi,0.,2.*TMath::Pi(),7,-0.5,6.5,nbins,pMin,pMax); 
    fOutputQA->Add(fAntiLambdaBckgNClustersITSPos);

    fAntiLambdaBckgNClustersITSNeg = 
      new TH3F("fAntiLambdaBckgNClustersITSNeg","#bar{#Lambda} Bckg: Neg. Daug;#phi;Num. ITS Clusters;;p_{T} (GeV/c)",nbinsPhi,0.,2.*TMath::Pi(),7,-0.5,6.5,nbins,pMin,pMax); 
    fOutputQA->Add(fAntiLambdaBckgNClustersITSNeg);

  }


  // ============================================================= //

  PostData(1, fOutput);
  PostData(2, fOutputQA);
  
}

//___________________________________________________________________________________________

static Bool_t AcceptTrack(const AliAODTrack *t) 
{
  // Track criteria for primaries particles 
    if (TMath::Abs(t->Eta())>0.8 )  return kFALSE; 
    if (!(t->TestFilterMask(1<<7))) return kFALSE; 

  return kTRUE;   
}


//___________________________________________________________________________________________

static Bool_t AcceptTrackV0(const AliAODTrack *t) 
{ 
  // Track criteria for daughter particles of V0 candidate 
  if (!t->IsOn(AliAODTrack::kTPCrefit)) return kFALSE;
  Float_t nCrossedRowsTPC = t->GetTPCClusterInfo(2,1); 
  if (nCrossedRowsTPC < 70) return kFALSE;

  return kTRUE;   
}


//___________________________________________________________________________________________

Bool_t AliAnalysisTaskLambdaOverK0sJets::AcceptV0(AliAODVertex *vtx, const AliAODv0 *v1) 
{ 
  // Selection for accepting V0 candidates 

  if (v1->GetOnFlyStatus()) return kFALSE;
  
  //if (v1->Pt() < pMin) return kFALSE; ***
  
  const AliAODTrack *ntrack1=(AliAODTrack *)v1->GetDaughter(1);
  const AliAODTrack *ptrack1=(AliAODTrack *)v1->GetDaughter(0);
    
  if( !ntrack1 || !ptrack1 ) return kFALSE;
  if( !AcceptTrackV0(ntrack1) ) return kFALSE;
  if( !AcceptTrackV0(ptrack1) ) return kFALSE;
  
  if( ntrack1->Charge() == ptrack1->Charge()) 
    return kFALSE;

  // Daughters: pseudo-rapidity cut
  if ( TMath::Abs(ntrack1->Eta()) > fMaxEtaDaughter  ||
       TMath::Abs(ptrack1->Eta()) > fMaxEtaDaughter  )
    return kFALSE;

  // Daughters: transverse momentum cut
  if ( ( ntrack1->Pt() < fMinPtDaughter ) || 
       ( ptrack1->Pt() < fMinPtDaughter )  ) 
    return kFALSE;
  
  // Daughters: Impact parameter of daughter to prim vtx
  Float_t xy = v1->DcaNegToPrimVertex();
  if (TMath::Abs(xy)<fDCAToPrimVtx) return kFALSE;
  xy = v1->DcaPosToPrimVertex();
  if (TMath::Abs(xy)<fDCAToPrimVtx) return kFALSE;

  // Daughters: DCA
  Float_t dca = v1->DcaV0Daughters();
  if (dca>fMaxDCADaughter) return kFALSE;

  // V0: Cosine of the pointing angle
  Float_t cpa=v1->CosPointingAngle(vtx);
  if (cpa<fMinCPA) return kFALSE;

  // V0: Fiducial volume
  Double_t xyz[3]; v1->GetSecondaryVtx(xyz);
  Float_t r2=xyz[0]*xyz[0] + xyz[1]*xyz[1];
  if (r2<0.9*0.9) return kFALSE;
  if (r2>lMax*lMax) return kFALSE;

  return kTRUE;
}

//___________________________________________________________________________________________

static Float_t dPHI(Float_t phi1, Float_t phi2) 
{ 
  // Calculate the phi difference between two tracks  
  Float_t deltaPhi = phi1 - phi2;
  
  if (deltaPhi<-TMath::PiOver2())    deltaPhi = deltaPhi + 2*(TMath::Pi());
  if (deltaPhi>(3*TMath::PiOver2()))  deltaPhi = deltaPhi - 2*(TMath::Pi());
  return deltaPhi;
}

//___________________________________________________________________________________________

static Float_t MyRapidity(Float_t rE, Float_t rPz)
{ 
  // Local method for rapidity
  return 0.5*TMath::Log((rE+rPz)/(rE-rPz+1.e-13));
} 

//___________________________________________________________________________________________

static Int_t EqualPt(AliAODTrack *trk, const AliAODTrack *nTrk, const AliAODTrack *pTrk)
{ 
  // Local method to compaire the momentum between two tracks

  //double const kEpsilon = 0.000001;
  Int_t    isSamePt = 0;

 /*
  Float_t p[3];     trk->GetPxPyPz(p);
  Float_t pNegTrk[3]; nTrk->GetPxPyPz(pNegTrk);
  Float_t pPosTrk[3]; pTrk->GetPxPyPz(pPosTrk);
  
 
  if( (  fabs(p[0]-pNegTrk[0])<kEpsilon && 
	 fabs(p[1]-pNegTrk[1])<kEpsilon && 
	 fabs(p[2]-pNegTrk[2])<kEpsilon ) 
      ||
      (  fabs(p[0]-pPosTrk[0])<kEpsilon && 
	 fabs(p[1]-pPosTrk[1])<kEpsilon && 
	 fabs(p[2]-pPosTrk[2])<kEpsilon ) )   
    isSamePt = 1;
  */
 
   
   if(  (TMath::Abs(nTrk->GetID())+1)==(TMath::Abs(trk->GetID()))  ||
       (TMath::Abs(pTrk->GetID())+1)==(TMath::Abs(trk->GetID())) )  isSamePt = 1;
  
  /*
  if(  (TMath::Abs(nTrk->GetID()))==(TMath::Abs(trk->GetID()))  ||
       (TMath::Abs(pTrk->GetID()))==(TMath::Abs(trk->GetID())) )  isSamePt = 1;
  */

  return isSamePt;

}

//___________________________________________________________________________________________

void AliAnalysisTaskLambdaOverK0sJets::RecCascade(AliAODTrack *trk1,const AliAODTrack *trk2,const AliAODTrack *trkBch,TString histo)
{
  // Local method to reconstruct cascades candidates from the combinations of three tracks
  // The input tracks correspond to the trigger particle and the daughter tracks of the V0 candidate (correlation step)
  // The trigger particle track will be always consider as a possible daughter of the V0 which coming from the Cascade decay.
  // The daughters of the V0 candidates are switched to be the bachelor track for the Cascade reconstruction.

  Float_t lMassBach=0., lPtot2Bach=0., lEBach=0.;
  Float_t lMassLambda=0., lPtot2Lambda=0., lELambda = 0.; 
  Float_t pLambda[3] = {0.,0.,0.};
  Float_t pCascade[3] = {0.,0.,0.};
  Float_t lMassCascade = 0., lPtot2Cascade=0.;

  // Two loops are done to consider the posibility to reconstruct a Xi or an Omega
  for(Int_t i=0;i<2;i++){

    // 0. Check the charge for both tracks: trk1 & trk2. 
    //    Usefull in the Lambda step.
    if( trk1->Charge() == trk2->Charge() ) 
      continue;
   
    // 1. Bachelor: Allocation for the track
    if(i==0) // Xi 
      lMassBach = TDatabasePDG::Instance()->GetParticle(kPiMinus)->Mass();
    else if(i==1) //Omega
      lMassBach = TDatabasePDG::Instance()->GetParticle(kKMinus)->Mass();

    lPtot2Bach = TMath::Power(trkBch->P(),2);

    lEBach = TMath::Sqrt(lPtot2Bach + lMassBach*lMassBach);

    // 2. Lambda: Kinematical properties
    lMassLambda = TDatabasePDG::Instance()->GetParticle(kLambda0)->Mass();
      
    pLambda[0] = trk1->Px() + trk2->Px();
    pLambda[1] = trk1->Py() + trk2->Py();
    pLambda[2] = trk1->Pz() + trk2->Pz();

    lPtot2Lambda = pLambda[0]*pLambda[0] +  pLambda[1]*pLambda[1] +  pLambda[2]*pLambda[2];

    lELambda = TMath::Sqrt(lPtot2Lambda + lMassLambda*lMassLambda);

    // 3. Cascade: Reconstruction
    pCascade[0] = pLambda[0] + trkBch->Px();
    pCascade[1] = pLambda[1] + trkBch->Py();
    pCascade[2] = pLambda[2] + trkBch->Pz();

    lPtot2Cascade = pCascade[0]*pCascade[0] + pCascade[1]*pCascade[1] + pCascade[2]*pCascade[2];

    lMassCascade = TMath::Sqrt( TMath::Power(lEBach+lELambda,2) - lPtot2Cascade );
   
    // 4. Filling histograms
    if( histo.Contains("K0s") ) {
      if(i==0) // Xi 
	fK0sMassCascade->Fill(lMassCascade,1);
      else if(i==1) //Omega
	fK0sMassCascade->Fill(lMassCascade,3);
    }
    else if( histo.Contains("Lambda") ) {
      if(i==0) // Xi 
	fLambdaMassCascade->Fill(lMassCascade,1);
      else if(i==1) //Omega
	fLambdaMassCascade->Fill(lMassCascade,3);
    }


  }

}

//___________________________________________________________________________________________
 
void AliAnalysisTaskLambdaOverK0sJets::V0Loop(V0LoopStep_t step, Bool_t isTriggered) 
{ 
  // Three options for the 'step' variable:
  // 1) TriggerCheck
  // 2) Reconstruction and Correlation
  // 3) Mixed event

  AliAODTrack *trkTrig = 0x0; 
  Float_t ptTrig  = -100.;
  Float_t phiTrig = -100.;
  Float_t etaTrig = -100.; 

  if( step==kTriggerCheck || isTriggered ){
    fIsTrigFromV0daug = 0;

    trkTrig = (AliAODTrack*)fAOD->GetTrack(fIdTrigger); 
    ptTrig  = trkTrig->Pt();
    phiTrig = trkTrig->Phi();
    etaTrig = trkTrig->Eta();
  }
  
  // *************************************************
  // Centrality selection
  AliCentrality *cent = fAOD->GetCentrality();
  Float_t centrality = cent->GetCentralityPercentile("V0M");

  // *************************************************
  // MC Event
  TClonesArray *stackMC = 0x0;
  Float_t mcXv=0., mcYv=0., mcZv=0.;
   
  if(fIsMC){
    TList *lst = fAOD->GetList();
    stackMC = (TClonesArray*)lst->FindObject(AliAODMCParticle::StdBranchName());
    if (!stackMC) {
      Printf("ERROR: stack not available");
    }

    AliAODMCHeader *mcHdr = 
      (AliAODMCHeader*)lst->FindObject(AliAODMCHeader::StdBranchName());
    
    mcXv=mcHdr->GetVtxX(); mcYv=mcHdr->GetVtxY(); mcZv=mcHdr->GetVtxZ();
  }
  
  // *************************************************
  // V0 loop - AOD
  const AliAODVertex *vtx=fAOD->GetPrimaryVertex();
  Float_t xv=vtx->GetX(), yv=vtx->GetY(), zv=vtx->GetZ();
  Int_t nV0sTot = fAOD->GetNumberOfV0s();
  for (Int_t iV0 = 0; iV0 < nV0sTot; iV0++) {
    
    AliAODv0 *v0=fAOD->GetV0(iV0);
    if (!v0) continue;
    if (!AcceptV0(fAOD->GetPrimaryVertex(),v0)) continue;
    
    const AliAODTrack *ntrack=(AliAODTrack *)v0->GetDaughter(1);
    const AliAODTrack *ptrack=(AliAODTrack *)v0->GetDaughter(0);

    // Decay vertex
    Double_t xyz[3]; v0->GetSecondaryVtx(xyz);
    Float_t dx=xyz[0]-xv, dy=xyz[1]-yv, dz=xyz[2]-zv;
    // Decay lenght: 2D & 3D 
    Float_t lt=TMath::Sqrt(dx*dx + dy*dy); 
    Float_t dl=TMath::Sqrt(dx*dx + dy*dy + dz*dz);  
    // Momentum: 2D & 3D
    Float_t pt=TMath::Sqrt(v0->Pt2V0());
    Float_t p=v0->P();

    /*
    Float_t dlK = 0.4977*lt/pt;
    Float_t dlL = 1.1157*lt/pt; 
    */
    Float_t dlK  = v0->MassK0Short()*dl/p;
    Float_t dlL  = v0->MassLambda()*dl/p;
    Float_t dlAL = v0->MassAntiLambda()*dl/p;

    // ctau
    Bool_t ctK=kTRUE;  if (dlK > fMaxCtau*2.68  || dlK < fMinCtau*2.68)  ctK=kFALSE; 
    Bool_t ctL=kTRUE;  if (dlL > fMaxCtau*7.89  || dlL < fMinCtau*7.89)  ctL=kFALSE; 
    Bool_t ctAL=kTRUE; if (dlAL > fMaxCtau*7.89 || dlAL < fMinCtau*7.89) ctAL=kFALSE;    

    // Armenteros variables:
    Float_t lAlphaV0      =  v0->AlphaV0();
    Float_t lPtArmV0      =  v0->PtArmV0();
    
    // MC Association:
    Bool_t lComeFromSigma     = kFALSE; 
    Bool_t lCheckMcK0Short    = kFALSE;
    Bool_t lCheckMcLambda     = kFALSE;
    Bool_t lCheckMcAntiLambda = kFALSE;
	
    Int_t lMCAssocNegDaug = 0;
    Int_t lMCAssocPosDaug = 0;  
    
    // ********* MC - Association *********
    // In case of injected-MC, the correlations might be done with only natural particles 
    Bool_t isNaturalPart = kTRUE;
    if(step==kCorrelation){ //Correlation
      
      if(fIsMC){        
      	if(!stackMC) goto noas;

	Int_t ntrkMC=stackMC->GetEntriesFast();
	
	Int_t nlab = TMath::Abs(ntrack->GetLabel());//** UInt_t
	Int_t plab = TMath::Abs(ptrack->GetLabel());
  
	// To avoid futher problems 
	if ( (nlab<0 || plab<0) ||
	     (nlab>=ntrkMC || plab>=ntrkMC) )
	  goto noas;      

	AliAODMCParticle *nPart=(AliAODMCParticle*)stackMC->UncheckedAt(nlab);
	AliAODMCParticle *pPart=(AliAODMCParticle*)stackMC->UncheckedAt(plab);
	// MC origin of daughters: Primaries?
	if( nPart->IsPhysicalPrimary() ) lMCAssocNegDaug = 1;
	if( pPart->IsPhysicalPrimary() ) lMCAssocPosDaug = 1;

	if(!nPart || !pPart)   goto noas;
	
	if ( TMath::Abs(nPart->Eta()) > fMaxEtaDaughter ||
	     TMath::Abs(pPart->Eta()) > fMaxEtaDaughter )
	  goto noas;
	
	// Daughter momentum cut
	if ( ( nPart->Pt() < fMinPtDaughter ) || 
	     ( pPart->Pt() < fMinPtDaughter )  ) 
	  goto noas;
	
	// ----------------------------------------
	
	Int_t lPDGCodeNegDaughter = nPart->GetPdgCode();
	Int_t lPDGCodePosDaughter = pPart->GetPdgCode();
	
	Int_t ipMother = pPart->GetMother();
	Int_t inMother = nPart->GetMother();
	
	if(inMother<0 || inMother>=ntrkMC) lMCAssocNegDaug = 5;
	if(ipMother<0 || ipMother>=ntrkMC) lMCAssocPosDaug = 5;

	if(inMother<0 || inMother>=ntrkMC) {  goto noas;}
	if(inMother != ipMother) { // did the negative daughter decay ?
	  AliAODMCParticle *negMotherOfMotherPart = (AliAODMCParticle*)stackMC->UncheckedAt(inMother);
	  if (negMotherOfMotherPart->GetMother() != ipMother) 
	    goto noas;
	}
	
	if (ipMother<0 || ipMother>=ntrkMC)
	  goto noas;     
	
	AliAODMCParticle *p0=(AliAODMCParticle*)stackMC->UncheckedAt(ipMother);
	if(!p0) 
	  goto noas; 

	// ----------------------------------------
	
	if ( (ipMother>=fEndOfHijingEvent) && 
	     (fEndOfHijingEvent!=-1)     && 
	     (p0->GetMother()<0) ) 
	  isNaturalPart = kFALSE; 
	else  isNaturalPart = kTRUE; 

	// ----------------------------------------
	
	if(fSeparateInjPart && !isNaturalPart) goto noas;     
	
	Int_t lPDGCodeV0 = p0->GetPdgCode();
	
	// MC origin of daughters:
	//Decay from Weak Decay?
	if( (TMath::Abs(lPDGCodeV0) == kK0Short) || (TMath::Abs(lPDGCodeV0) == kLambda0) || 
	    (TMath::Abs(lPDGCodeV0) == kSigmaMinus) || (TMath::Abs(lPDGCodeV0) == kSigmaPlus) ||
	    (TMath::Abs(lPDGCodeV0) == kSigma0) || (TMath::Abs(lPDGCodeV0) == kXiMinus) ||
	    (TMath::Abs(lPDGCodeV0) == kOmegaMinus) )
	  { lMCAssocNegDaug = 2; 	  lMCAssocPosDaug = 2; }
	// Gamma conversion
	else if( TMath::Abs(lPDGCodeV0) == kGamma )
	  { lMCAssocNegDaug = 3; 	  lMCAssocPosDaug = 3; }
	// Unidentied mother:
	else 
	  { lMCAssocNegDaug = 4; 	  lMCAssocPosDaug = 4; }


	Int_t lIndexMotherOfMother   = p0->GetMother();
	Int_t lPdgcodeMotherOfMother = 0;
	if (lIndexMotherOfMother ==-1) lPdgcodeMotherOfMother = 0;
	else {
	  AliAODMCParticle *lMCAODMotherOfMother=(AliAODMCParticle*)stackMC->UncheckedAt(lIndexMotherOfMother);
	  if (!lMCAODMotherOfMother) {lPdgcodeMotherOfMother=0;}
	  lPdgcodeMotherOfMother = lMCAODMotherOfMother->GetPdgCode();
	}
	
	// Daughter momentum cut: ! FIX it in case of AOD ! //MC or REc
	if ( (nPart->Pt()  < fMinPtDaughter ) ||
	     (pPart->Pt()  < fMinPtDaughter ) )
	  goto noas;
	
	if( (lPDGCodeV0 != kK0Short) &&
	    (lPDGCodeV0 != kLambda0) &&
	    (lPDGCodeV0 != kLambda0Bar) ) 
	  goto noas;
	

	lComeFromSigma     = kFALSE; 
	lCheckMcK0Short    = kFALSE;
	lCheckMcLambda     = kFALSE;
	lCheckMcAntiLambda = kFALSE;
	     
	// ----------------------------------------
      
	// K0s
	if( (lPDGCodePosDaughter==+211) && (lPDGCodeNegDaughter==-211) &&
	    (inMother==ipMother) && (lPDGCodeV0==310) ) {
	  
	  if ( ((AliAODMCParticle*)stackMC->UncheckedAt(ipMother))->IsPrimary()  )
	    lCheckMcK0Short  = kTRUE;
	  
	}
	// Lambda
	else if( (lPDGCodePosDaughter==+2212) && (lPDGCodeNegDaughter==-211)  &&
		 (inMother==ipMother) && (lPDGCodeV0==3122)  ){
	  
	  if ( ( TMath::Abs(lPdgcodeMotherOfMother) == 3212) ||
	       ( TMath::Abs(lPdgcodeMotherOfMother) == 3224) ||
	       ( TMath::Abs(lPdgcodeMotherOfMother) == 3214) ||
	       ( TMath::Abs(lPdgcodeMotherOfMother) == 3114)
	       ) lComeFromSigma = kTRUE;
	  else lComeFromSigma = kFALSE; 
	  
	  if ( ((AliAODMCParticle*)stackMC->UncheckedAt(ipMother))->IsPrimary() || 
	       ( !(((AliAODMCParticle*)stackMC->UncheckedAt(ipMother))->IsPrimary() ) 
		 && (lComeFromSigma==kTRUE) )
	       ) lCheckMcLambda  = kTRUE; 
	  
	}
	// AntiLambda
	else if( (lPDGCodePosDaughter==211) && (lPDGCodeNegDaughter==-2212) &&
		 (inMother==ipMother) && (lPDGCodeV0==-3122) ) {
	  
	  
	  if ( ( TMath::Abs(lPdgcodeMotherOfMother) == 3212) ||
	       ( TMath::Abs(lPdgcodeMotherOfMother) == 3224) ||
	       ( TMath::Abs(lPdgcodeMotherOfMother) == 3214) ||
	       ( TMath::Abs(lPdgcodeMotherOfMother) == 3114)
	       ) lComeFromSigma = kTRUE;
	  else lComeFromSigma = kFALSE;  
	  
	  if ( ((AliAODMCParticle*)stackMC->UncheckedAt(ipMother))->IsPrimary() || 
	       ( (!((AliAODMCParticle*)stackMC->UncheckedAt(ipMother))->IsPrimary()) 
		 && (lComeFromSigma==kTRUE) )
	       ) lCheckMcAntiLambda  = kTRUE;
	  
	}
	
	//  ----------------------------------------
	
	if ((p0->Pt())<pMin) goto noas;
	if (TMath::Abs(p0->Y())>fYMax ) goto noas;
	
	Float_t dxAs = mcXv - p0->Xv(),  dyAs = mcYv - p0->Yv(),  dzAs = mcZv - p0->Zv();
	Float_t l = TMath::Sqrt(dxAs*dxAs + dyAs*dyAs + dzAs*dzAs);
	
	dxAs = mcXv - pPart->Xv(); dyAs = mcYv - pPart->Yv();
	//Float_t ltAs = TMath::Sqrt(dxAs*dxAs + dyAs*dyAs);
	Float_t ptAs = p0->Pt();
	Float_t rapAs = p0->Y();
	Float_t etaAs = p0->Eta();
	// phi resolution for V0-reconstruction
	Float_t resPhi = p0->Phi() - v0->Phi();	

	if (l < 0.01) { // Primary V0
	  
	  // K0s:
	  if(ctK && lCheckMcK0Short){ 
	    fK0sAssocPt->Fill(ptAs);
	    fK0sAssocPtRap->Fill(ptAs,rapAs,centrality);
	    if(centrality<10)
	      fK0sAssocPtPhiEta->Fill(p0->Phi(),etaAs,ptAs);
	    
	    if(lPtArmV0 > TMath::Abs(0.2*lAlphaV0) )
	      fK0sAssocPtArm->Fill(ptAs,rapAs,centrality);

	    fK0sMCResPhi->Fill(resPhi,pt,centrality);
	  } // End K0s selection
	  // Lambda:
	  else if(ctL && lCheckMcLambda) {    
	    fLambdaAssocPt->Fill(ptAs);
	    fLambdaAssocPtRap->Fill(ptAs,rapAs,centrality);
	    if(centrality<10)
	      fLambdaAssocPtPhiEta->Fill(p0->Phi(),etaAs,ptAs);

	    fLambdaMCResPhi->Fill(resPhi,pt,centrality);
	  }// End Lambda:
	  // AntiLambda:
	  else if (ctAL && lCheckMcAntiLambda){
	    fAntiLambdaAssocPt->Fill(ptAs);
	    fAntiLambdaAssocPtRap->Fill(ptAs,rapAs,centrality);
	    if(centrality<10)
	      fAntiLambdaAssocPtPhiEta->Fill(p0->Phi(),etaAs,ptAs);

	    fAntiLambdaMCResPhi->Fill(resPhi,pt,centrality);
	  } // End AntiLambda:
	  
	} // End Primary V0 selection
	
	// After the kinematical selection of K0s and Lambdas
	// it might be that the daugthers are not identified through MC Association
	if(lMCAssocNegDaug==0)
	  lMCAssocNegDaug = 5;
	if(lMCAssocPosDaug==0)
	  lMCAssocPosDaug = 5;
	
	
      } // End MC-Association 
      
      // ************************************
      
    }// End Correlation Step
    
  noas:

    Float_t pPos = -100.;
    Float_t pNeg = -100.;
    Float_t dedxPos = -1000.;
    Float_t dedxNeg = -1000.;

    Float_t nsigPosPion   = 0.;
    Float_t nsigPosProton = 0.;
    Float_t nsigNegPion   = 0.;
    Float_t nsigNegProton = 0.;

    if(fUsePID && !fIsMC) {     
      const AliAODPid *pidNeg = ntrack->GetDetPid();
      const AliAODPid *pidPos = ptrack->GetDetPid();
      
      if (pidNeg && pidPos) {
	pPos = pidPos->GetTPCmomentum();
	pNeg = pidNeg->GetTPCmomentum();
	dedxPos = pidPos->GetTPCsignal()/47.; 
	dedxNeg = pidNeg->GetTPCsignal()/47.; 
    
	if(pPos<1.){
	  nsigPosPion   = TMath::Abs(fPIDResponse->NumberOfSigmasTPC(ptrack,AliPID::kPion));
	  nsigPosProton = TMath::Abs(fPIDResponse->NumberOfSigmasTPC(ptrack,AliPID::kProton));
	}
	if(pNeg<1.){
	  nsigNegPion   = TMath::Abs(fPIDResponse->NumberOfSigmasTPC(ntrack,AliPID::kPion));
	  nsigNegProton = TMath::Abs(fPIDResponse->NumberOfSigmasTPC(ntrack,AliPID::kProton));
	}

      }

    }

    Float_t dcaNeg = -100.;
    Float_t dcaPos = -100.;
    Float_t lPtNeg = -100.;
    Float_t lPtPos = -100.;
    Float_t phiNeg = -100.;
    Float_t phiPos = -100.;
    Float_t etaNeg = -100.;
    Float_t etaPos = -100.;

    Float_t dPtPos = -100.;
    Float_t dPtNeg = -100.;

    Float_t dca   = -100;
    Float_t cpa   = -100;
    Float_t lEta  = -100.;
    Float_t lPhi  = -100.;
    
    Float_t radio = -100.;
    Float_t dPhi  = -100.;
    Float_t dEta  = -100.; 

     /*
    // Good regions
    lPhi  = v0->Phi();
    if(lPhi>0. && lPhi<1.8) continue;
    if(lPhi>2.2 && lPhi<3.8) continue;
    if(lPhi>4.2 && lPhi<5.8) continue;
    */

    /*
    // Bad regions
    lPhi  = v0->Phi();
    if(lPhi>1.8 && lPhi<2.2) continue;
    if(lPhi>3.8 && lPhi<4.2) continue;
    if(lPhi>5.8 && lPhi<6.2) continue;
    */

    if(step==kCorrelation || fDoQA){//Correlation
      dcaNeg = v0->DcaNegToPrimVertex();
      dcaPos = v0->DcaPosToPrimVertex();
      lPtNeg = ntrack->Pt();
      lPtPos = ptrack->Pt();
      phiPos = ptrack->Phi();
      phiNeg = ntrack->Phi();
      etaPos = ptrack->Eta();
      etaNeg = ntrack->Eta();

      dPtPos = pt - lPtPos;
      dPtNeg = pt - lPtNeg;

      dca   = v0->DcaV0Daughters();
      cpa   = v0->CosPointingAngle(fAOD->GetPrimaryVertex());
      lEta  = v0->PseudoRapV0();
      lPhi  = v0->Phi();
      lPhi  = ( (lPhi < 0) ? lPhi + 2*TMath::Pi() : lPhi );

      dPhi  = dPHI(phiTrig,lPhi);
      dEta  = etaTrig - v0->PseudoRapV0();    
      radio = TMath::Sqrt(dPhi*dPhi + dEta*dEta);
    }

    // Comparing the pt of the trigger particle wrt the v0-candidate's daughter:
    // It is used as well for the side-band subtraction
    Int_t isSameTrk = -1;
    if( step==kTriggerCheck || isTriggered )
      isSameTrk = EqualPt(trkTrig,ntrack,ptrack);
    
    // *******************
    // Disentangle the V0 candidate

    Float_t massK0s = 0., mK0s = 0., sK0s = 0.;
    Float_t massLambda = 0., mLambda = 0., sLambda = 0.;
    Float_t massAntiLambda = 0.;

    Bool_t isCandidate2K0s = kFALSE;
    massK0s = v0->MassK0Short();
    mK0s = TDatabasePDG::Instance()->GetParticle(kK0Short)->Mass();
    sK0s = 0.0044 + (0.008-0.0044)/(10-1)*(pt - 1.);
    /*
    if (ctK && (TMath::Abs(v0->RapK0Short())<fYMax) &&
	(nsigNegPion < fNSigma) && (nsigPosPion < fNSigma) &&
	(TMath::Abs(mK0s-massK0s) < 3*sK0s) )
    */
    if ( TMath::Abs(mK0s-massK0s) < 3*sK0s )
      isCandidate2K0s = kTRUE;     
    
    Bool_t isCandidate2Lambda = kFALSE;
    massLambda = v0->MassLambda();
    mLambda = TDatabasePDG::Instance()->GetParticle(kLambda0)->Mass();
    //s=0.0027 + (0.004-0.0027)/(10-1)*(pt-1);
    //s=0.0015 + (0.002-0.0015)/(2.6-1)*(pt-1);
    sLambda=0.0023 + (0.004-0.0023)/(6-1)*(pt-1);
    /*
    if (ctL && (TMath::Abs(v0->RapLambda())<fYMax)  &&
	(nsigNegPion < fNSigma)   &&
	(nsigPosProton < fNSigma) &&
    	(TMath::Abs(mLambda-massLambda) < 3*sLambda) )
    */
    if (TMath::Abs(mLambda-massLambda) < 3*sLambda)
      isCandidate2Lambda = kTRUE;  
    
    Bool_t isCandidate2LambdaBar = kFALSE;
    massAntiLambda = v0->MassAntiLambda();
    /*
    if (ctL && (TMath::Abs(v0->RapLambda())<fYMax)  &&
	(nsigPosPion < fNSigma)   &&
	(nsigNegProton < fNSigma) &&
	(TMath::Abs(mLambda-massAntiLambda) < 3*sLambda) )
    */
    if (TMath::Abs(mLambda-massAntiLambda) < 3*sLambda)
      isCandidate2LambdaBar = kTRUE; 

    
    // *******************
    //   K0s selection
    // *******************
    if (ctK && (TMath::Abs(v0->RapK0Short())<fYMax) &&
        ( lPtArmV0 > TMath::Abs(0.2*lAlphaV0) ) ) {
      
      switch(step) {
      case kTriggerCheck: 

	if (isCandidate2K0s){
	  if(pt>ptTrig){
	    fIsV0LP = 1; 
	    fPtV0LP = pt;
	  }       
	  
	  fIsTrigFromV0daug = isSameTrk;
	  if(fIsTrigFromV0daug){
	    Printf("  The LP has the same momentum in X and Y as one of the K0s daughters *** iV0 %d",iV0); 


	    if(fCheckIDTrig){
              Printf("  The LP has the same momentum in X and Y as one of the K0s daughters *** iV0 %d \n\t\t %d %d %d \n\t\t %lf %lf %lf \n\t\t %lf %lf %lf \n\t\t %lf %lf \n\t\t %lf %lf ",
		     iV0, TMath::Abs( trkTrig->GetID() ),
                     ntrack->GetID() ,  ptrack->GetID() ,
                     TMath::Abs( ntrack->Px() - trkTrig->Px() ), TMath::Abs(  ntrack->Py() - trkTrig->Py() ), TMath::Abs( ntrack->Pz() - trkTrig->Pz() ),
                     TMath::Abs( ptrack->Px() - trkTrig->Px() ), TMath::Abs(  ptrack->Py() - trkTrig->Py() ), TMath::Abs( ptrack->Pz() - trkTrig->Pz() ),
                     TMath::Abs( ntrack->Phi() - trkTrig->Phi() ), TMath::Abs(  ntrack->Eta() - trkTrig->Eta() ),
                     TMath::Abs( ptrack->Phi() - trkTrig->Phi() ), TMath::Abs(  ptrack->Eta() - trkTrig->Eta() )
                     );
          

              Float_t posDeltaPt =  ptTrig - ptrack->Pt();
              Float_t negDeltaPt =  ptTrig - ntrack->Pt();

              Float_t posDeltaPhi =  phiTrig - ptrack->Phi();
              Float_t negDeltaPhi =  phiTrig - ntrack->Phi();

	      	      
	      if(  (TMath::Abs(ptrack->GetID())+1)==(TMath::Abs(trkTrig->GetID())) ){
		fCheckIDTrigPtK0s->Fill(posDeltaPt,0.,pt); 
		fCheckIDTrigPhiK0s->Fill(posDeltaPhi,0.,pt);
	      }
	      else if( (TMath::Abs(ntrack->GetID())+1)==(TMath::Abs(trkTrig->GetID())) ){ 
		fCheckIDTrigPtK0s->Fill(negDeltaPt,2.,pt); 
		fCheckIDTrigPhiK0s->Fill(negDeltaPhi,2.,pt);
	      }
	      
	    } // End check ID
	  } // Close isTrigFromV0daug

	} 

	break; // End K0s selection for TriggerCheck
      case kCorrelation:

	fK0sMass->Fill(massK0s,pt,0);
	fK0sPtvsEta->Fill(pt,lEta,0);
	fK0sPtvsRap->Fill(pt,v0->RapK0Short(),0);
	if(centrality<10) fK0sMass->Fill(massK0s,pt,1);
	
	if(pt>2. && pt<5.) fK0sMassPtPhi->Fill(massK0s,pt,lPhi);

	// Only for triggered events and in case of MC K0s is not a embeded particle
	if( isTriggered && isNaturalPart ){
	  
	  fK0sMass->Fill(massK0s,pt,3);
	  fK0sPtvsEta->Fill(pt,lEta,1);
	  fK0sPtvsRap->Fill(pt,v0->RapK0Short(),1);
	  
	  fK0sMassPtvsPtL->Fill(massK0s,pt,ptTrig);  //****
	  if(centrality<10) fK0sMass->Fill(massK0s,pt,4);
	  
	}
	// Invariant Mass cut
	if (TMath::Abs(mK0s-massK0s) < 3*sK0s) {
	
	  if(fDoQA){ // Quality Assurance
	    fK0sPIDPosDaug->Fill(pPos,dedxPos,pt);
	    fK0sPIDNegDaug->Fill(pNeg,dedxNeg,pt);
	    
	    fK0sPhiEtaPosDaug->Fill(phiPos,etaPos,pt);
	    fK0sPhiEtaNegDaug->Fill(phiNeg,etaNeg,pt);
	    
	    fK0sDCAPosDaug->Fill(dcaPos,pt);
	    fK0sDCANegDaug->Fill(dcaNeg,pt);
	    
	    fK0sDifPtPosDaug->Fill(dPtPos,pt);
	    fK0sDifPtNegDaug->Fill(dPtNeg,pt);
	    
	    fK0sDecayPos->Fill(dx,dy,pt);
	    fK0sDecayVertex->Fill(lt,pt);
	    
	    if(lt<1.0)
	      fK0sDecayVertexZoom->Fill(lt,pt); //**
	    fK0sCPA->Fill(cpa,pt); //**
	    fK0sDCAV0Daug->Fill(dca,pt); //**

	  }// End QA

	  fK0sPtLtSB->Fill(pt,lt);
	  fK0sEtaPhi->Fill(lPhi,lEta);
	  fK0sPtvsEta->Fill(pt,lEta,2);
	  fK0sPtvsRap->Fill(pt,v0->RapK0Short(),2);
		  
	  // ==== Correlations ==== //
	  if( isTriggered && isNaturalPart ){

	    fHistArmenterosPodolanski->Fill(lAlphaV0,lPtArmV0,0);
	    fK0sPtvsEta->Fill(pt,lEta,3);
	    fK0sPtvsRap->Fill(pt,v0->RapK0Short(),3);
	    fK0sdPhiPtAssocPtL->Fill(dPhi,pt,ptTrig);

	    // *** pt bin and centrality
	    for(Int_t k=0;k<kN1;k++)
	      if( (pt>kPtBinV0[k]) && (pt<kPtBinV0[k+1]) ){
		fK0sdPhidEtaPtL[k]->Fill(dPhi,dEta,zv);

		if( TMath::Abs(etaTrig)<0.5 )
		  fK0sdPhidEtaPtL2[k]->Fill(dPhi,dEta,zv);
		
		if(centrality<10){
		  fK0sdPhidEtaPtLCent[k]->Fill(dPhi,dEta,zv);
		  if( TMath::Abs(etaTrig)<0.5 )
		    fK0sdPhidEtaPtLCent2[k]->Fill(dPhi,dEta,zv);
		}

		if(radio < 0.4)
		  fK0sDCADaugToPrimVtx->Fill(dcaPos,dcaNeg,ptTrig);	    
		
	      } // End selection pt bin
	    
	  } // End triggered selection
	  
	} // End selection in mass
	
	if( TMath::Abs(mK0s-massK0s + 6.5*sK0s) < 1.5*sK0s ||
	    TMath::Abs(mK0s-massK0s - 6.5*sK0s) < 1.5*sK0s  ) {
	  
	  if(fDoQA){ // Quality Assurance
	      fK0sBckgPIDPosDaug->Fill(pPos,dedxPos,pt);
	      fK0sBckgPIDNegDaug->Fill(pNeg,dedxNeg,pt);
            
	      fK0sBckgPhiEtaPosDaug->Fill(phiPos,etaPos,pt);
	      fK0sBckgPhiEtaNegDaug->Fill(phiNeg,etaNeg,pt);
	      
	      fK0sBckgDCAPosDaug->Fill(dcaPos,pt);
	      fK0sBckgDCANegDaug->Fill(dcaNeg,pt);
	      
	      fK0sBckgDifPtPosDaug->Fill(dPtPos,pt);
	      fK0sBckgDifPtNegDaug->Fill(dPtNeg,pt);
	      
	      fK0sBckgDecayPos->Fill(dx,dy,pt);
	      fK0sBckgDecayVertex->Fill(lt,pt);
	      
	      if(lt<1.0)
		fK0sBckgDecayVertexZoom->Fill(lt,pt); //**
	      fK0sBckgCPA->Fill(cpa,pt); //**
	      fK0sBckgDCAV0Daug->Fill(dca,pt); //**
	      
	  } // End QA

	  fK0sEtaPhi->Fill(lPhi,lEta,-1);
	  fK0sPtLtSB->Fill(pt,lt,-1);
		
	  // ==== Correlations ==== //
	  if( !isSameTrk && isTriggered && isNaturalPart){
	    
	    fHistArmenterosPodolanski->Fill(lAlphaV0,lPtArmV0,1);
	    fK0sdPhiPtAssocPtL->Fill(dPhi,pt,ptTrig,-1);
  
	    // Pt bin & centrality
	    for(Int_t k=0;k<kN1;k++)
	      if( (pt>kPtBinV0[k]) && (pt<kPtBinV0[k+1]) ){
		
		fK0sdPhidEtaPtLBckg[k]->Fill(dPhi,dEta,zv);
		if( TMath::Abs(etaTrig)<0.5 )
		  fK0sdPhidEtaPtLBckg2[k]->Fill(dPhi,dEta,zv);

		if(centrality<10){
		  fK0sdPhidEtaPtLCentBckg[k]->Fill(dPhi,dEta,zv);
		  if( TMath::Abs(etaTrig)<0.5 )
		    fK0sdPhidEtaPtLCentBckg2[k]->Fill(dPhi,dEta,zv);
		}

		if(radio < 0.4){ // Under the correlation peak
		  fHistArmPodBckg->Fill(lAlphaV0,lPtArmV0,0);
		  fK0sBckgDecLength->Fill(dlK,ptTrig);
		  fK0sBckgDCADaugToPrimVtx->Fill(dcaPos,dcaNeg,ptTrig);
		  fK0sBckgEtaPhi->Fill(lPhi,lEta);
		  fK0sBckgPhiRadio->Fill(lPhi,lt);

		  // MC Association of daughter particles 
		  for(Int_t ii=0;ii<5;ii++){
		    if(lMCAssocNegDaug == ii){
		      fK0sBckgDCANegDaugToPrimVtx->Fill(ii,dcaNeg);
		      //if eta selection for the trigger particle
		    }
		    if(lMCAssocPosDaug == ii){
		      fK0sBckgDCAPosDaugToPrimVtx->Fill(ii,dcaPos);
		      //if eta selection for the trigger particle
		    }

		  }

		  RecCascade(trkTrig,ntrack,ptrack,"K0s");
		  RecCascade(trkTrig,ptrack,ntrack,"K0s");

		}// End selection in the correlation peak
		
	      } // End selection pt bin
	    
	  } // End triggered selection
	  
	}// End selection in outside the mass cut
	
	break; // End K0s selection for Corrleation
      default:
	Printf( " Selection of 'step' is not set properly");
	break;
	
      }// End switch

    } // End K0s selection

    // *******************
    // Lambda selection
    // *******************
    if ( ctL && (TMath::Abs(v0->RapLambda())<fYMax)  &&
	 (nsigPosProton < fNSigma) ){

      
      switch(step) {
      case kTriggerCheck: 
	
	if (isCandidate2Lambda){
	  if(pt>ptTrig) {
	    fIsV0LP = 1;
	    fPtV0LP = pt;
	  }

	  fIsTrigFromV0daug = isSameTrk;
	  if(fIsTrigFromV0daug){
	    Printf("  The LP has the same momentum in X and Y as one of the Lambda daughters *** iV0 %d",iV0); 

	    
	    if(fCheckIDTrig){
              Printf("  The LP has the same momentum in X and Y as one of the L daughters *** iV0 %d \n\t\t %d %d %d \n\t\t %lf %lf %lf \n\t\t %lf %lf %lf \n\t\t %lf %lf \n\t\t %lf %lf ",
                   iV0, TMath::Abs( trkTrig->GetID() ),
                    ntrack->GetID() ,  ptrack->GetID() ,
                   TMath::Abs( ntrack->Px() - trkTrig->Px() ), TMath::Abs(  ntrack->Py() - trkTrig->Py() ), TMath::Abs( ntrack->Pz() - trkTrig->Pz() ),
                   TMath::Abs( ptrack->Px() - trkTrig->Px() ), TMath::Abs(  ptrack->Py() - trkTrig->Py() ), TMath::Abs( ptrack->Pz() - trkTrig->Pz() ),
                   TMath::Abs( ntrack->Phi() - trkTrig->Phi() ), TMath::Abs(  ntrack->Eta() - trkTrig->Eta() ),
                   TMath::Abs( ptrack->Phi() - trkTrig->Phi() ), TMath::Abs(  ptrack->Eta() - trkTrig->Eta() )
                   );
          
	      
              Float_t posDeltaPt =  ptTrig - ptrack->Pt();
              Float_t negDeltaPt =  ptTrig - ntrack->Pt();

              Float_t posDeltaPhi =  phiTrig - ptrack->Phi();
              Float_t negDeltaPhi =  phiTrig - ntrack->Phi();

	      if(  (TMath::Abs(ptrack->GetID())+1)==(TMath::Abs(trkTrig->GetID())) ){
		fCheckIDTrigPtLambda->Fill(posDeltaPt,0.,pt); 
		fCheckIDTrigPhiLambda->Fill(posDeltaPhi,0.,pt);
	      }
	      else if( (TMath::Abs(ntrack->GetID())+1)==(TMath::Abs(trkTrig->GetID())) ){
		fCheckIDTrigPtLambda->Fill(negDeltaPt,2.,pt); 
		fCheckIDTrigPhiLambda->Fill(negDeltaPhi,2.,pt);
	      }
	    
	    } // End check ID
	  } // Close isTrigFromV0daug

	} 
	
	break; // End Lambda selection for TriggerCheck
      case kCorrelation:
	
	fLambdaMass->Fill(massLambda,pt,0);
	fLambdaPtvsEta->Fill(pt,lEta,0);
	fLambdaPtvsRap->Fill(pt,v0->RapLambda(),0);
	if(centrality<10) fLambdaMass->Fill(massLambda,pt,1);

	if(pt>2. && pt<5.) fLambdaMassPtPhi->Fill(massLambda,pt,lPhi);

	// Only for triggered events and in case of MC Lambda is not a embeded particle
	if( isTriggered && isNaturalPart ){
	  
	  fLambdaMass->Fill(massLambda,pt,3);
	  fLambdaPtvsEta->Fill(pt,lEta,1);
	  fLambdaPtvsRap->Fill(pt,v0->RapLambda(),1);
	  
	  fLambdaMassPtvsPtL->Fill(massLambda,pt,ptTrig);
	  if(centrality<10)  fLambdaMass->Fill(massLambda,pt,4);
	  
	} 
	// Invariant Mass cut
	if (TMath::Abs(mLambda-massLambda) < 3*sLambda) {

	  if(fDoQA){ // Quality Assurance
	    fLambdaPIDPosDaug->Fill(pPos,dedxPos,pt);
	    fLambdaPIDNegDaug->Fill(pNeg,dedxNeg,pt);
          
	    fLambdaPhiEtaPosDaug->Fill(phiPos,etaPos,pt);
	    fLambdaPhiEtaNegDaug->Fill(phiNeg,etaNeg,pt);

	    fLambdaDCAPosDaug->Fill(dcaPos,pt);
	    fLambdaDCANegDaug->Fill(dcaNeg,pt);

	    fLambdaDifPtPosDaug->Fill(dPtPos,pt);
	    fLambdaDifPtNegDaug->Fill(dPtNeg,pt);

	    fLambdaDecayPos->Fill(dx,dy,pt);
	    fLambdaDecayVertex->Fill(lt,pt);

	    if(lt<1.0)
	      fLambdaDecayVertexZoom->Fill(lt,pt); //**
	    fLambdaCPA->Fill(cpa,pt); //**
	    fLambdaDCAV0Daug->Fill(dca,pt); //**

	  } //End QA

	  fLambdaPtLtSB->Fill(pt,lt);	
	  fLambdaEtaPhi->Fill(lPhi,lEta);
	  fLambdaPtvsEta->Fill(pt,lEta,2);
	  fLambdaPtvsRap->Fill(pt,v0->RapLambda(),2);
	  
	  // ==== Correlations ==== //
	  if( isTriggered && isNaturalPart ){
	    fHistArmenterosPodolanski->Fill(lAlphaV0,lPtArmV0,2);
	    fLambdaPtvsEta->Fill(pt,lEta,3);
	    fLambdaPtvsRap->Fill(pt,v0->RapLambda(),3);
	    fLambdadPhiPtAssocPtL->Fill(dPhi,pt,ptTrig);
	
	    // *** pt bin and centrality
	    for(Int_t k=0;k<kN1;k++)
	      if( (pt>kPtBinV0[k]) && (pt<kPtBinV0[k+1]) ){
		fLambdadPhidEtaPtL[k]->Fill(dPhi,dEta,zv);
		
		if( TMath::Abs(etaTrig)<0.5 )
		  fLambdadPhidEtaPtL2[k]->Fill(dPhi,dEta,zv);

		  if(centrality<10){
		    fLambdadPhidEtaPtLCent[k]->Fill(dPhi,dEta,zv);
		    if( TMath::Abs(etaTrig)<0.5 )
		      fLambdadPhidEtaPtLCent2[k]->Fill(dPhi,dEta,zv);
		  }
		  
		if(radio < 0.4)
		  fLambdaDCADaugToPrimVtx->Fill(dcaPos,dcaNeg,ptTrig);

	      } // End selection pt bin
	    
	  } // End triggered selection
	  
	} // End selection in mass
	
	if( (TMath::Abs(mLambda-massLambda + 6.5*sLambda) < 1.5*sLambda) ||
	    (TMath::Abs(mLambda-massLambda - 6.5*sLambda) < 1.5*sLambda) ){
	  
	  if(fDoQA){ // Quality Assurance
	    fLambdaBckgPIDPosDaug->Fill(pPos,dedxPos,pt);
	    fLambdaBckgPIDNegDaug->Fill(pNeg,dedxNeg,pt);
          
	    fLambdaBckgPhiEtaPosDaug->Fill(phiPos,etaPos,pt);
	    fLambdaBckgPhiEtaNegDaug->Fill(phiNeg,etaNeg,pt);

	    fLambdaBckgDCAPosDaug->Fill(dcaPos,pt);
	    fLambdaBckgDCANegDaug->Fill(dcaNeg,pt);

	    fLambdaBckgDifPtPosDaug->Fill(dPtPos,pt);
	    fLambdaBckgDifPtNegDaug->Fill(dPtNeg,pt);

	    fLambdaBckgDecayPos->Fill(dx,dy,pt);
	    fLambdaBckgDecayVertex->Fill(lt,pt);

	    if(lt<1.0)
	      fLambdaBckgDecayVertexZoom->Fill(lt,pt); //**
	    fLambdaBckgCPA->Fill(cpa,pt); //**
	    fLambdaBckgDCAV0Daug->Fill(dca,pt); //**

	  }

	  fLambdaPtLtSB->Fill(pt,lt,-1);
	  fLambdaEtaPhi->Fill(lPhi,lEta,-1);

	  // ==== Correlations ==== //
	  if( !isSameTrk && isTriggered && isNaturalPart ){
	    
	    fHistArmenterosPodolanski->Fill(lAlphaV0,lPtArmV0,3);
	  
	    // Pt bin & centrality
	    for(Int_t k=0;k<kN1;k++)
	      if( (pt>kPtBinV0[k]) && (pt<kPtBinV0[k+1]) ){
		
		fLambdadPhidEtaPtLBckg[k]->Fill(dPhi,dEta,zv);
		if( TMath::Abs(etaTrig)<0.5 )
		  fLambdadPhidEtaPtLBckg2[k]->Fill(dPhi,dEta,zv);

		if(centrality<10){
		  fLambdadPhidEtaPtLCentBckg[k]->Fill(dPhi,dEta,zv);
		  if( TMath::Abs(etaTrig)<0.5 )
		    fLambdadPhidEtaPtLCentBckg2[k]->Fill(dPhi,dEta,zv);
		}

		if(radio < 0.4){ // Under the peak
		  fHistArmPodBckg->Fill(lAlphaV0,lPtArmV0,1);
		  fLambdaBckgDecLength->Fill(dlL,ptTrig);
		  fLambdaBckgDCADaugToPrimVtx->Fill(dcaPos,dcaNeg,ptTrig);
		  fLambdaBckgEtaPhi->Fill(lPhi,lEta);
		  fLambdaBckgPhiRadio->Fill(lPhi,lt);
		  
		  // MC Association of daughter particles 
		  for(Int_t ii=0;ii<5;ii++){
		    if(lMCAssocNegDaug == ii)
		      fLambdaBckgDCANegDaugToPrimVtx->Fill(ii,dcaNeg);
		    
		    if(lMCAssocPosDaug == ii)
		      fLambdaBckgDCAPosDaugToPrimVtx->Fill(ii,dcaPos);
		  }

		  RecCascade(trkTrig,ntrack,ptrack,"Lambda");
		  RecCascade(trkTrig,ptrack,ntrack,"Lambda");

		}// End selection in the correlation peak
		
	      }// End selection pt bi

	  }// End triggered selection
	  
	}// End selection in outside the mass cut

	break; // End Lambda selection for Correlation
      default:
	Printf(" Selection of 'step' is not set properly");
	break;
	
      }// End switch
      
    } // End Lambda selection

    // *******************
    // AntiLambda selection
    // *******************
    if ( ctL && (TMath::Abs(v0->RapLambda())<fYMax)  &&
	 (nsigNegProton < fNSigma)  ) {
      
      switch(step) {
      case kTriggerCheck: 
	
	if (isCandidate2LambdaBar){
	  if(pt>ptTrig) {
	    fIsV0LP = 1;
	    fPtV0LP = pt;
	  }
	  
	  fIsTrigFromV0daug = isSameTrk;
	  if(fIsTrigFromV0daug){
	    Printf("  The LP has the same momentum in X and Y as one of the AntiLambda daughters *** iV0 %d",iV0); 

	    if(fCheckIDTrig){
	      Printf("  The LP has the same momentum in X and Y as one of the AL daughters *** iV0 %d \n\t\t %d %d %d \n\t\t %lf %lf %lf \n\t\t %lf %lf %lf \n\t\t %lf %lf \n\t\t %lf %lf ",
		     iV0, TMath::Abs( trkTrig->GetID() ),
		     ntrack->GetID() ,  ptrack->GetID() ,
		     TMath::Abs( ntrack->Px() - trkTrig->Px() ), TMath::Abs(  ntrack->Py() - trkTrig->Py() ), TMath::Abs( ntrack->Pz() - trkTrig->Pz() ),
		     TMath::Abs( ptrack->Px() - trkTrig->Px() ), TMath::Abs(  ptrack->Py() - trkTrig->Py() ), TMath::Abs( ptrack->Pz() - trkTrig->Pz() ),
		     TMath::Abs( ntrack->Phi() - trkTrig->Phi() ), TMath::Abs(  ntrack->Eta() - trkTrig->Eta() ),
		     TMath::Abs( ptrack->Phi() - trkTrig->Phi() ), TMath::Abs(  ptrack->Eta() - trkTrig->Eta() )
		     );
	      
	    }// End CheckTrigger  

	  }
	  
	}
	break; // End AntiLambda selection for CheckTrigger
      case kCorrelation: 
	
	fAntiLambdaMass->Fill(massAntiLambda,pt,0);
	fAntiLambdaPtvsEta->Fill(pt,lEta,0);
	fAntiLambdaPtvsRap->Fill(pt,v0->RapLambda(),0);
	if(centrality<10) fAntiLambdaMass->Fill(massAntiLambda,pt,1);

	if(pt>2. && pt<5.) fAntiLambdaMassPtPhi->Fill(massAntiLambda,pt,lPhi);

	// Only for triggered events and in case of MC AntiLambda is not a embeded particle
	if( isTriggered && isNaturalPart ){
	  
	  fAntiLambdaMass->Fill(massAntiLambda,pt,3);
	  fAntiLambdaPtvsEta->Fill(pt,lEta,1);
	  fAntiLambdaPtvsRap->Fill(pt,v0->RapLambda(),1);
	  
	  fAntiLambdaMassPtvsPtL->Fill(massAntiLambda,pt,ptTrig);
	  if(centrality<10)  fAntiLambdaMass->Fill(massAntiLambda,pt,4);
	  
	} 
	// Invariant Mass cut
	if (TMath::Abs(mLambda-massAntiLambda) < 3*sLambda) {

	  if(fDoQA){ // Quality Assurance
	    fAntiLambdaPIDPosDaug->Fill(pPos,dedxPos,pt);
	    fAntiLambdaPIDNegDaug->Fill(pNeg,dedxNeg,pt);
          
	    fAntiLambdaPhiEtaPosDaug->Fill(phiPos,etaPos,pt);
	    fAntiLambdaPhiEtaNegDaug->Fill(phiNeg,etaNeg,pt);

	    fAntiLambdaDCAPosDaug->Fill(dcaPos,pt);
	    fAntiLambdaDCANegDaug->Fill(dcaNeg,pt);

	    fAntiLambdaDifPtPosDaug->Fill(dPtPos,pt);
	    fAntiLambdaDifPtNegDaug->Fill(dPtNeg,pt);

	    fAntiLambdaDecayPos->Fill(dx,dy,pt);
	    fAntiLambdaDecayVertex->Fill(lt,pt);

	    if(lt<1.0)
	      fAntiLambdaDecayVertexZoom->Fill(lt,pt); //**
	    fAntiLambdaCPA->Fill(cpa,pt); //**
	    fAntiLambdaDCAV0Daug->Fill(dca,pt); //**

	  } //End QA

	  fAntiLambdaPtLtSB->Fill(pt,lt);	
	  fAntiLambdaEtaPhi->Fill(lPhi,lEta);
	  fAntiLambdaPtvsEta->Fill(pt,lEta,2);
	  fAntiLambdaPtvsRap->Fill(pt,v0->RapLambda(),2);
	  
	  // ==== Correlations ==== //
	  if( isTriggered && isNaturalPart ){
	    fHistArmenterosPodolanski->Fill(lAlphaV0,lPtArmV0,4);
	    fAntiLambdaPtvsEta->Fill(pt,lEta,3);
	    fAntiLambdaPtvsRap->Fill(pt,v0->RapLambda(),3);
	    
	    fAntiLambdadPhiPtAssocPtL->Fill(dPhi,pt,ptTrig);
	
	    // *** pt bin and centrality
	    for(Int_t k=0;k<kN1;k++)
	      if( (pt>kPtBinV0[k]) && (pt<kPtBinV0[k+1]) ){
		fAntiLambdadPhidEtaPtL[k]->Fill(dPhi,dEta,zv);
		
		if( TMath::Abs(etaTrig)<0.5 )
		  fAntiLambdadPhidEtaPtL2[k]->Fill(dPhi,dEta,zv);

		if(centrality<10){
		  fAntiLambdadPhidEtaPtLCent[k]->Fill(dPhi,dEta,zv);
		  if( TMath::Abs(etaTrig)<0.5 )
		    fAntiLambdadPhidEtaPtLCent2[k]->Fill(dPhi,dEta,zv);
		}
		  
		if(radio < 0.4)
		  fAntiLambdaDCADaugToPrimVtx->Fill(dcaPos,dcaNeg,ptTrig);

	      } // End selection pt bin
	    
	  } // End triggered selection
	  
	} // End selection in mass
	
	if( (TMath::Abs(mLambda-massAntiLambda + 6.5*sLambda) < 1.5*sLambda) ||
	    (TMath::Abs(mLambda-massAntiLambda - 6.5*sLambda) < 1.5*sLambda) ){
	  
	  if(fDoQA){ // Quality Assurance
	    fAntiLambdaBckgPIDPosDaug->Fill(pPos,dedxPos,pt);
	    fAntiLambdaBckgPIDNegDaug->Fill(pNeg,dedxNeg,pt);
          
	    fAntiLambdaBckgPhiEtaPosDaug->Fill(phiPos,etaPos,pt);
	    fAntiLambdaBckgPhiEtaNegDaug->Fill(phiNeg,etaNeg,pt);

	    fAntiLambdaBckgDCAPosDaug->Fill(dcaPos,pt);
	    fAntiLambdaBckgDCANegDaug->Fill(dcaNeg,pt);

	    fAntiLambdaBckgDifPtPosDaug->Fill(dPtPos,pt);
	    fAntiLambdaBckgDifPtNegDaug->Fill(dPtNeg,pt);

	    fAntiLambdaBckgDecayPos->Fill(dx,dy,pt);
	    fAntiLambdaBckgDecayVertex->Fill(lt,pt);

	    if(lt<1.0)
	      fAntiLambdaBckgDecayVertexZoom->Fill(lt,pt); //**
	    fAntiLambdaBckgCPA->Fill(cpa,pt); //**
	    fAntiLambdaBckgDCAV0Daug->Fill(dca,pt); //**

	  } // End QA

	  fAntiLambdaPtLtSB->Fill(pt,lt,-1);
	  fAntiLambdaEtaPhi->Fill(lPhi,lEta,-1);

	  // ==== Correlations ==== //
	  if( !isSameTrk && isTriggered && isNaturalPart ){
	    
	    fHistArmenterosPodolanski->Fill(lAlphaV0,lPtArmV0,5);
	    
	    // Pt bin & centrality
	    for(Int_t k=0;k<kN1;k++)
	      if( (pt>kPtBinV0[k]) && (pt<kPtBinV0[k+1]) ){
		
		fAntiLambdadPhidEtaPtLBckg[k]->Fill(dPhi,dEta,zv);
		if( TMath::Abs(etaTrig)<0.5 )
		  fAntiLambdadPhidEtaPtLBckg2[k]->Fill(dPhi,dEta,zv);

		if(centrality<10){
		  fAntiLambdadPhidEtaPtLCentBckg[k]->Fill(dPhi,dEta,zv);
		  if( TMath::Abs(etaTrig)<0.5 )
		    fAntiLambdadPhidEtaPtLCentBckg2[k]->Fill(dPhi,dEta,zv);
		}

		if(radio < 0.4){ // Under the peak
		  fHistArmPodBckg->Fill(lAlphaV0,lPtArmV0,2);
		  fAntiLambdaBckgDecLength->Fill(dlL,ptTrig);
		  fAntiLambdaBckgDCADaugToPrimVtx->Fill(dcaPos,dcaNeg,ptTrig);
		  fAntiLambdaBckgEtaPhi->Fill(lPhi,lEta);
		  fAntiLambdaBckgPhiRadio->Fill(lPhi,lt);
		  
		  // MC Association of daughter particles 
		  for(Int_t ii=0;ii<5;ii++){
		    if(lMCAssocNegDaug == ii)
		      fAntiLambdaBckgDCANegDaugToPrimVtx->Fill(ii,dcaNeg);
		    
		    if(lMCAssocPosDaug == ii)
		      fAntiLambdaBckgDCAPosDaugToPrimVtx->Fill(ii,dcaPos);
		  }

		  //RecCascade(trkTrig,ntrack,ptrack,"AntiLambda");
		  //RecCascade(trkTrig,ptrack,ntrack,"AntiLambda");

		}// End selection in the correlation peak
		
	      }// End selection pt bi

	  }// End triggered selection
	  
	}// End selection in outside the mass cut
	
	break;
      default:
	Printf( " Selection of 'step' is not set properly");
	break;
      }// End switch
      
    } // End AntiLambda selection
      
  } // End V0 loop
  
}

//___________________________________________________________________________________________

void AliAnalysisTaskLambdaOverK0sJets::TriggerParticle() 
{ 
  // Obtain the trigger particle of the event to perform the correlations in phi and eta

  Float_t  ptTrigger  = -1000.;
  Int_t    idSndTrigger = 0;

  // ----------------------------
  // 1. First trigger particle 

  Int_t nTrk= fAOD->GetNumberOfTracks();
  for (Int_t i=0; i<nTrk; i++) {

    AliAODTrack *t = fAOD->GetTrack(i);
    if(!AcceptTrack(t)) continue;
    Float_t pt=t->Pt();
	
    if(pt>ptTrigger) {
      ptTrigger = pt;
      fIdTrigger = i;
    }
    
  }

  if(fIdTrigger<0) 
    { fEvents->Fill(7); return; }
   
  // ----------------------------
  // 2. Checking if the trigger particle 
  // might be a daughter from the V0-candidate
  V0Loop(kTriggerCheck,kFALSE);
    
  fCheckTriggerFromV0Daug->Fill(fIsTrigFromV0daug);
  // V0-candidate is the highest particle in the event:
  if(fIsV0LP) fTriggerIsV0->Fill(fPtV0LP);
  // V0-candidate is the highest particle in the event & the trigger-particle is its daughter:
  if(fIsTrigFromV0daug && fIsV0LP)  fCheckTriggerFromV0Daug->Fill(2);
  if(fIsTrigFromV0daug) fTriggerComingFromDaug->Fill(ptTrigger);
  else return;

  // ----------------------------
  // 3. Second trigger particle: when isV0LP is true
  fIsTrigFromV0daug = 0;
  fIsSndCheck = 1; 
  ptTrigger = -1000.;
 
  for (Int_t i=0; i<nTrk; i++) {
    if(i == fIdTrigger) continue;

    AliAODTrack *t=fAOD->GetTrack(i);
    if(!AcceptTrack(t)) continue;
    Float_t pt=t->Pt();

    if(pt>ptTrigger) {
      ptTrigger = pt;
      idSndTrigger = i;
    }
    
  }

  fIdTrigger = idSndTrigger;

  // ----------------------------
  // 3. Checking if the trigger second particle 
  // might be a daughter from the V0-candidate
  V0Loop(kTriggerCheck,kFALSE); 
  
  if(!fIsTrigFromV0daug) fCheckTriggerFromV0Daug->Fill(-1);
  if(fIsTrigFromV0daug && fIsV0LP) fCheckTriggerFromV0Daug->Fill(3);

}

//___________________________________________________________________________________________

void AliAnalysisTaskLambdaOverK0sJets::UserExec(Option_t *)
{
  // Main loop for the Analysis

  // Initializing global variables for the correlation studies (mandatory for each event).
  // ---- 1) Trigger Particle: id track
  fIdTrigger  = -1;
  // ---- 2) TriggerCheck: Variables used to crosscheck if trigger particle is a V0 daughter ---- //
  fIsTrigFromV0daug = 0; 
  fIsV0LP     = 0;
  fPtV0LP     = -10.;
  fIsSndCheck = 0;

  // Getting AOD Event
  fAOD = (AliAODEvent *)InputEvent();
  fEvents->Fill(0); //event counter  

  if (!fAOD) {
    Printf("ERROR: aod not available");
    return;
  }
  fEvents->Fill(1);
  
  // Physics selection
  AliAnalysisManager *mgr = AliAnalysisManager::GetAnalysisManager();
  AliInputEventHandler *hdr=(AliInputEventHandler*)mgr->GetInputEventHandler();
  UInt_t maskIsSelected = hdr->IsEventSelected();
  Bool_t isSelected = (maskIsSelected & AliVEvent::kMB);
  if (!isSelected) return;
  fEvents->Fill(2);

  // Centrality selection
  AliCentrality *cent = fAOD->GetCentrality();
  Float_t centrality = cent->GetCentralityPercentile("V0M");
  fCentrality->Fill(centrality);

  if (!cent->IsEventInCentralityClass(fCentMin,fCentMax,"V0M")) return;
  fEvents->Fill(3);

  // Primary vertex
  const AliAODVertex *vtx = fAOD->GetPrimaryVertex();
  if (vtx->GetNContributors()<3) return;
  fEvents->Fill(4);

  Float_t xv=vtx->GetX(), yv=vtx->GetY(), zv=vtx->GetZ();
  
  fPrimaryVertexX->Fill(xv);
  fPrimaryVertexY->Fill(yv);
  fPrimaryVertexZ->Fill(zv);

  if (TMath::Abs(zv) > 10.) return;   
  fEvents->Fill(5);

  // ***********************************************
  // Multiplicity with charged-tracks 

  fPIDResponse = hdr->GetPIDResponse(); 
  
  Int_t nTrk = fAOD->GetNumberOfTracks();
  Int_t mult=0;
  Float_t nsig=0;
  
  for (Int_t iTrk=0; iTrk<nTrk; iTrk++) {
    
    AliAODTrack *track = fAOD->GetTrack(iTrk);
    if(!AcceptTrack(track)) continue;
    mult++;

    if(fUsePID){
      const AliAODPid *pid = track->GetDetPid();
      if (!pid) continue;
      
      Float_t p = pid->GetTPCmomentum();
      Float_t dedx = pid->GetTPCsignal()/47.; //47?
      fdEdx->Fill(p,dedx,1);
      
      nsig = fPIDResponse->NumberOfSigmasTPC(track,AliPID::kProton);
      if ( TMath::Abs(nsig) < 3. )
	fdEdxPid->Fill(p,dedx,1);
    }

  }
  
  if( (mult<2300) && (centrality<2) ) fEvents->Fill(6);
  
  fCentMult->Fill(centrality,mult);
  
  // **********************************************
  // Triggered Particle -  Trigger Particle
  TriggerParticle();
   
  Bool_t isTriggered = kFALSE;
  Float_t ptTrigger  = -100.;
  Float_t phiTrigger = -100.;
  Float_t etaTrigger = -100.;
  AliAODTrack *trkTrigger = 0x0;

  if(fIdTrigger>=0){
    trkTrigger = (AliAODTrack*)fAOD->GetTrack(fIdTrigger);
    ptTrigger  = trkTrigger->Pt();
    phiTrigger = trkTrigger->Phi();
    etaTrigger = trkTrigger->Eta();
  }
  
  // -------------------
   
  // The highest-pt particle is a V0-candidate 
  if(fIsV0LP) { fEvents->Fill(8); }
  // The highest-pt particle is a V0-candidate's daughter  
  if(fIsTrigFromV0daug) { fEvents->Fill(9); }
  // The sencond highest-pt particle is a V0-candidate's daughter  
  if( fIsTrigFromV0daug && fIsSndCheck ) { fEvents->Fill(10); }
  // Trigger Particle Properties
  if( !fIsTrigFromV0daug &&  (TMath::Abs(etaTrigger)<fTrigEtaMax) ){
    fTriggerEtaPhi->Fill(phiTrigger,etaTrigger);
    fTriggerPtCent->Fill(ptTrigger,centrality,zv);
    // Event is triggered
    if( (ptTrigger>=fTrigPtMin)  && (ptTrigger<=fTrigPtMax) ) {
      fEvents->Fill(11); 
      isTriggered=kTRUE;
    }
    else
      fEvents->Fill(12);
  }
 
  // ******************************************
  // Start loop over MC particles
  
  fEndOfHijingEvent = -1;
  TClonesArray *stack = 0x0;
  Float_t mcXv=0., mcYv=0., mcZv=0.;
  
  if(fIsMC) {

    TList *lst = fAOD->GetList();
    stack = (TClonesArray*)lst->FindObject(AliAODMCParticle::StdBranchName());
    if (!stack) {
      Printf("ERROR: stack not available");
      return;
    }
    
    AliAODMCHeader *mcHdr = 
      (AliAODMCHeader*)lst->FindObject(AliAODMCHeader::StdBranchName());
  
    mcXv=mcHdr->GetVtxX(); mcYv=mcHdr->GetVtxY(); mcZv=mcHdr->GetVtxZ();
  
    Int_t nTrkMC = stack->GetEntriesFast();
    // -----------------------------------------
    // --------- Trigger particle --------------
    // -----------------------------------------

    Float_t triggerMCPt   = -1000.;
    Float_t triggerMCPhi  = -1000.;
    Float_t triggerMCEta  = -1000.;
    Bool_t  isTriggeredMC = kFALSE;

    for (Int_t iTrkMC = 0; iTrkMC < nTrkMC; iTrkMC++){
      
      AliAODMCParticle *p0 = (AliAODMCParticle*)stack->At(iTrkMC);
      if(!p0) continue;
      if(TMath::Abs(p0->Eta())>0.8) continue;
      if(p0->Pt()<0.15) continue;

      // ----------------------------------------
      
      // For injected MC: determine where HIJING event ends 
      if (fEndOfHijingEvent==-1) { 
        if ( ( p0->GetStatus() == 21 ) ||
	     ( (p0->GetPdgCode() == 443) &&
	       (p0->GetMother() == -1)   &&
	       (p0->GetDaughter(0) ==  (iTrkMC+1))) ) {
	  fEndOfHijingEvent = iTrkMC; 
        }
      }

      // ----------------------------------------
      
      Int_t isNaturalPart = 1;
      if ( (iTrkMC>=fEndOfHijingEvent) && 
	   (fEndOfHijingEvent!=-1)     && 
	   (p0->GetMother()<0) ) 
	isNaturalPart = 0; 
     
      // ----------------------------------------
      
      Int_t lPdgcodeCurrentPart = p0->GetPdgCode();       
      if ( (lPdgcodeCurrentPart == kK0Short) ||
	   (lPdgcodeCurrentPart == kLambda0) ||
	   (lPdgcodeCurrentPart == kLambda0Bar) ) continue;
      
      // ----------------------------------------
	
      if(isNaturalPart == 0) continue;
      if( !p0->IsPhysicalPrimary() ) continue;
      
      Float_t ptPrim = p0->Pt();
      if(ptPrim>triggerMCPt){

	triggerMCPt  = p0->Pt();
	triggerMCPhi = p0->Phi();
	triggerMCEta = p0->Eta();

      } // End trigger selection

    } // End loop over charged particles


    // ----------------------------------------------------------------
    // Check if Primary particle has larger pt than strange particles 
    // ----------------------------------------------------------------
    fEndOfHijingEvent = -1;
    Bool_t isV0LPMC = kFALSE;
    if(triggerMCPt>1.)
      for (Int_t iTrkMC = 0; iTrkMC < nTrkMC; iTrkMC++){
      
	AliAODMCParticle *p0 = (AliAODMCParticle*)stack->At(iTrkMC);
	if(!p0) continue;
	
	// ----------------------------------------

	Int_t lPdgcodeCurrentPart = p0->GetPdgCode();       
	if ( (lPdgcodeCurrentPart != kK0Short) &&
	     (lPdgcodeCurrentPart != kLambda0) &&
	     (lPdgcodeCurrentPart != kLambda0Bar) ) continue;
	
	// ----------------------------------------
	
	// For injected MC: determine where HIJING event ends 
	if (fEndOfHijingEvent==-1) { 
	  if ( ( p0->GetStatus() == 21 ) ||
	       ( (p0->GetPdgCode() == 443) &&
		 (p0->GetMother() == -1)   &&
		 (p0->GetDaughter(0) ==  (iTrkMC+1))) ) {
	    fEndOfHijingEvent = iTrkMC; 
	  }
	}

	// ----------------------------------------
	
	Int_t isNaturalPart = 1;
	if ( (iTrkMC>=fEndOfHijingEvent) && 
	     (fEndOfHijingEvent!=-1)     && 
	     (p0->GetMother()<0) ) 
	  isNaturalPart = 0; 
	
	if(fSeparateInjPart)
	  if(isNaturalPart == 0) continue;

	// ----------------------------------------
	
	Float_t lRapCurrentPart = MyRapidity(p0->E(),p0->Pz());      
	Float_t lPtCurrentPart  = p0->Pt();
	
	Int_t iCurrentMother = p0->GetMother();       
	AliAODMCParticle *pCurrentMother = (AliAODMCParticle *)stack->At(iCurrentMother);
	Int_t lPdgCurrentMother = 0;    
	if (iCurrentMother == -1) { lPdgCurrentMother = 0;}
	else { lPdgCurrentMother = pCurrentMother->GetPdgCode(); }
	
	Int_t id0  = p0->GetDaughter(0);
	Int_t id1  = p0->GetDaughter(1);
	
	//if ( id0 ==  id1 ) continue;
	if ( (id0 < 0 || id1 < 0) ||
	     (id0 >=nTrkMC  || id1 >= nTrkMC) ) continue;
	
	AliAODMCParticle *pDaughter0 = (AliAODMCParticle *)stack->UncheckedAt(id0);
	AliAODMCParticle *pDaughter1 = (AliAODMCParticle *)stack->UncheckedAt(id1);
	if (!pDaughter0 || !pDaughter1) continue; 
	
	if ( TMath::Abs(pDaughter0->Eta()) > fMaxEtaDaughter ||
	     TMath::Abs(pDaughter1->Eta()) > fMaxEtaDaughter )
	  continue;	

	// Daughter momentum cut: ! FIX it in case of AOD !
	if ( ( pDaughter0->Pt() < fMinPtDaughter ) || 
	     ( pDaughter1->Pt() < fMinPtDaughter )  ) 
	  continue;
	
	if ((p0->Pt())<pMin) continue;  
	if (TMath::Abs(lRapCurrentPart) > fYMax)  continue;
	
	Float_t dx = mcXv-p0->Xv(),  dy = mcYv-p0->Yv(),  dz = mcZv-p0->Zv();
	Float_t l = TMath::Sqrt(dx*dx + dy*dy + dz*dz);
	
	//Cut in the 3D-distance of the secondary vertex to primary vertex
	if (l > 0.01) continue; // secondary V0 
	
	// is V0 the LP?
	if(lPtCurrentPart>triggerMCPt)
	  isV0LPMC = kTRUE;	  

      }
    
    // If V0 is not the particle with highest pt
    // then the event is triggered
    if(!isV0LPMC)
      fTriggerMCPtCent->Fill(triggerMCPt,triggerMCEta,centrality);
    else
      fEvents->Fill(13);
    
    if( (triggerMCPt>=fTrigPtMin)  && (triggerMCPt<=fTrigPtMax) &&
	(TMath::Abs(triggerMCEta)<fTrigEtaMax) &&
	(isV0LPMC==kFALSE) ) 
      isTriggeredMC=kTRUE;


    // -----------------------------------------
    // ---------- Strange particles ------------
    // -----------------------------------------
    Float_t dPhiMC = -100.;
    Float_t dEtaMC = -100.;
    fEndOfHijingEvent = -1;
    for (Int_t iTrkMC = 0; iTrkMC < nTrkMC; iTrkMC++){
      
      AliAODMCParticle *p0 = (AliAODMCParticle*)stack->At(iTrkMC);
      if(!p0) continue;
      
      // ----------------------------------------

      Int_t lPdgcodeCurrentPart = p0->GetPdgCode();       
      if ( (lPdgcodeCurrentPart != kK0Short) &&
	   (lPdgcodeCurrentPart != kLambda0) &&
	   (lPdgcodeCurrentPart != kLambda0Bar) ) continue;
      
      // ----------------------------------------

      Int_t isNaturalPart = 1;
      if ( (iTrkMC>=fEndOfHijingEvent) && 
	   (fEndOfHijingEvent!=-1)     && 
	   (p0->GetMother()<0) ) 
	isNaturalPart = 0; 
      
      fInjectedParticles->Fill(isNaturalPart);

      if(fSeparateInjPart && !isNaturalPart) continue;

      // ----------------------------------------

      Float_t lRapCurrentPart = MyRapidity(p0->E(),p0->Pz());      
      Float_t lEtaCurrentPart = p0->Eta();
      Float_t lPhiCurrentPart = p0->Phi();
      Float_t lPtCurrentPart  = p0->Pt();

      Int_t iCurrentMother = p0->GetMother();       
      AliAODMCParticle *pCurrentMother = (AliAODMCParticle *)stack->At(iCurrentMother);
      Int_t lPdgCurrentMother = 0;    
      if (iCurrentMother == -1) { lPdgCurrentMother = 0;}
      else { lPdgCurrentMother = pCurrentMother->GetPdgCode(); }

      Int_t id0  = p0->GetDaughter(0);
      Int_t id1  = p0->GetDaughter(1);
    
      //if ( id0 ==  id1 ) continue;
      if ( (id0 < 0 || id1 < 0) ||
	   (id0 >=nTrkMC  || id1 >= nTrkMC) ) continue;

      AliAODMCParticle *pDaughter0 = (AliAODMCParticle *)stack->UncheckedAt(id0);
      AliAODMCParticle *pDaughter1 = (AliAODMCParticle *)stack->UncheckedAt(id1);
      if (!pDaughter0 || !pDaughter1) continue; 
   
      if ( TMath::Abs(pDaughter0->Eta()) > fMaxEtaDaughter ||
	   TMath::Abs(pDaughter1->Eta()) > fMaxEtaDaughter )
	continue;	

      // Daughter momentum cut: ! FIX it in case of AOD !
      if ( ( pDaughter0->Pt() < fMinPtDaughter ) || 
	   ( pDaughter1->Pt() < fMinPtDaughter )  ) 
	continue;
      
      if ((p0->Pt())<pMin) continue;  
      if (TMath::Abs(lRapCurrentPart) > fYMax)  continue;
    
      Float_t dx = mcXv-p0->Xv(),  dy = mcYv-p0->Yv(),  dz = mcZv-p0->Zv();
      Float_t l = TMath::Sqrt(dx*dx + dy*dy + dz*dz);
      
      //Cut in the 3D-distance of the secondary vertex to primary vertex
      if (l > 0.01) continue; // secondary V0 
     
      //Transverse distance to vertex
      dx = mcXv-pDaughter0->Xv(); dy = mcYv-pDaughter0->Yv();
      //Float_t lt=TMath::Sqrt(dx*dx + dy*dy);

      // K0s
      if (lPdgcodeCurrentPart == kK0Short) {

	fK0sMCPt->Fill(lPtCurrentPart);
	fK0sMCPtRap->Fill(lPtCurrentPart,lRapCurrentPart,centrality);

	if(centrality<10)
	  fK0sMCPtPhiEta->Fill(lPhiCurrentPart,lEtaCurrentPart,lPtCurrentPart);

	// Triggered Event
	if(isTriggeredMC){
	  dPhiMC = dPHI(triggerMCPhi,lPhiCurrentPart);
	  dEtaMC = triggerMCEta - lEtaCurrentPart;
	  
	  // Pt bin
	  for(Int_t k=0;k<kN1;k++)
	    if( (lPtCurrentPart>kPtBinV0[k]) && (lPtCurrentPart<kPtBinV0[k+1]) ){	      
	      fK0sdPhidEtaMC[k]->Fill(dPhiMC,dEtaMC,zv);
	      
	      if(centrality<10)
		fK0sdPhidEtaMCCent[k]->Fill(dPhiMC,dEtaMC,zv);
	
	    } // End pt bin
	  
	} // End trigger event

      } // End K0s selection
      
      // Lambda
      if (lPdgcodeCurrentPart == kLambda0) {

	fLambdaMCPt->Fill(lPtCurrentPart);
	fLambdaMCPtRap->Fill(lPtCurrentPart,lRapCurrentPart,centrality);

	if(centrality<10)
	  fLambdaMCPtPhiEta->Fill(lPhiCurrentPart,lEtaCurrentPart,lPtCurrentPart);

	// Triggered Event
	if(isTriggeredMC){

	  dPhiMC = dPHI(triggerMCPhi,lPhiCurrentPart);
	  dEtaMC = triggerMCEta - lEtaCurrentPart;
	  
	  // Pt bin
	  for(Int_t k=0;k<kN1;k++)
	    if( (lPtCurrentPart>kPtBinV0[k]) && (lPtCurrentPart<kPtBinV0[k+1]) ){
     
	      fLambdadPhidEtaMC[k]->Fill(dPhiMC,dEtaMC,zv);
	          
	      if(centrality<10)
		fLambdadPhidEtaMCCent[k]->Fill(dPhiMC,dEtaMC,zv);
	
	    } //End pt bin
	  
	} // End trigger event

      } // End Lambda
      // AntiLambda
      if (lPdgcodeCurrentPart == kLambda0Bar) {

	fAntiLambdaMCPt->Fill(lPtCurrentPart);
	fAntiLambdaMCPtRap->Fill(lPtCurrentPart,lRapCurrentPart,centrality);

	if(centrality<10)
	  fAntiLambdaMCPtPhiEta->Fill(lPhiCurrentPart,lEtaCurrentPart,lPtCurrentPart);

	// Triggered Event
	if(isTriggeredMC){

	  dPhiMC = dPHI(triggerMCPhi,lPhiCurrentPart);
	  dEtaMC = triggerMCEta - lEtaCurrentPart;
	  
	  // Pt bin
	  for(Int_t k=0;k<kN1;k++)
	    if( (lPtCurrentPart>kPtBinV0[k]) && (lPtCurrentPart<kPtBinV0[k+1]) ){
     
	      fAntiLambdadPhidEtaMC[k]->Fill(dPhiMC,dEtaMC,zv);
	          
	      if(centrality<10)
		fAntiLambdadPhidEtaMCCent[k]->Fill(dPhiMC,dEtaMC,zv);
	
	    } //End pt bin
	  
	} // End trigger event

      } // End AntiLambda
     
    } // End loop over MC
    
  } // End MC condition

  // *************************************************
  // V0 loop - AOD
  V0Loop(kCorrelation,isTriggered);

}

//___________________________________________________________________________________________

void AliAnalysisTaskLambdaOverK0sJets::Terminate(Option_t *)
{
  // The Terminate() function is the last function to be called during
  // a query. It always runs on the client, it can be used to present
  // the results graphically or save the results to file.
  
  fOutput=(TList*)GetOutputData(1);
  if (fOutput) {
    Printf("\n\t *** DONE: fOutput available *** \n");
    return;
  }
  else{
    Printf("ERROR: fOutput not available");
    return;
  }
   
}
