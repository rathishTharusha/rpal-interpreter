# **Architecture Design and Implementation of the Antigravity Multi-Agent Workflow**

## **The Evolution of Event-Driven Multi-Agent Systems**

The proliferation of large language models has fundamentally altered the software development life cycle, transitioning the industry from isolated, single-turn code generation to complex, event-driven multi-agent systems. In sophisticated development environments, managing the cognitive load of autonomous agents, optimizing token economics, and maintaining execution fidelity requires a highly structured, state-driven architecture. The integration of a Tiered Human-in-the-Loop (HITL) workflow combined with Markdown-Driven Memory and Model Routing serves as a robust foundational paradigm for mitigating the inherent unpredictability of autonomous coding entities. By formalizing state transitions through explicit documentation checkpoints, the system creates deterministic boundaries that ground the probabilistic nature of large language models.  
The Antigravity architecture operates on a principle of cognitive routing, where computational tasks are systematically segmented according to their requisite reasoning depth and contextual requirements. High-tier models, inherently designed for complex reasoning, architectural synthesis, and cross-modal analysis, are strictly reserved for the Planning and Critic phases. Conversely, highly localized, context-aware execution is delegated to low-cost, high-speed worker models. This separation of concerns strictly adheres to the principle of scope management, wherein the models are fed manageable, explicitly defined tasks rather than attempting to ingest and mutate the entirety of a vast codebase simultaneously.  
The introduction of multiple HITL review stages acts as a critical epistemic alignment mechanism. In contemporary software engineering, the speed of code generation often eclipses the speed of architectural alignment, leading to scenarios where autonomous systems rapidly build divergent minimum viable products based on conflicting assumptions. By enforcing human validation at both the planning stage and the final knowledge transfer stage, the Antigravity system prevents the rapid generation of divergent code, ensuring that the initial specifications are fully synchronized with developer intent before any computational execution resources are expended.

## **Markdown-Driven Memory and Epistemic Synchronization**

A persistent challenge in the deployment of autonomous agents is epistemic drift—the phenomenon wherein an agent's internal context window diverges from the objective ground truth of the external environment over sequential iterations. Multi-agent systems that attempt to maintain state purely within the active context window eventually succumb to context degradation and hallucination as the token limit is approached. To resolve this, the Antigravity architecture leverages Markdown-Driven Memory, externalizing the system state into persistent file structures that serve as the single source of truth.  
The macro-state of the project is maintained within the ARCHITECTURE.md file. This document acts as the global knowledge base, detailing high-level goals, invariants, structural dependencies, and approaches to avoid. When the Orchestrator agent initializes, it reads this document to establish its foundational context, treating it as a literal brain dump of everything the model must know before reasoning about new code.  
Conversely, the micro-state of active execution is managed via the PLAN\_TICKET.md file. This file functions as a deterministic state machine transition vector. It breaks down the broader implementation requirements into logical, bite-sized tasks and milestones, functioning as an automated design document. By explicitly segregating the macro-state from the micro-state, the architecture ensures that the execution agents operate with focused, narrow scopes, minimizing the probability of unauthorized refactoring or feature creep during the tool execution loop.

## **Advanced Token Optimization via Context Caching**

Operating multi-agent systems over large codebases introduces a severe economic bottleneck: the exponential escalation of token processing costs. In standard API interactions, transmitting a large document, a complete repository, or extensive conversation history to a model incurs billing for all input tokens on every single request. If an orchestrator queries a codebase across fifty sequential tool-calling iterations, the baseline context is processed and billed fifty times. Context caching algorithms resolve this inefficiency by allowing developers to store the context once and reference it in subsequent requests, paying exclusively for the cached storage duration and the unique query tokens appended during each iteration.

### **Implicit Versus Explicit Caching Mechanisms**

The underlying architecture relies on the caching protocols provided by the Gemini API, which offers two distinct operational modalities: implicit caching and explicit caching.  
Implicit caching is a background optimization automatically enabled on recent model architectures (such as Gemini 2.5 and newer models). When a system transmits a sufficiently large prompt, the infrastructure attempts to cache the common prefix. If a subsequent request shares this identical prefix within a short temporal window, the request benefits from a cache hit. However, implicit caching offers no cost-saving guarantees and provides no programmatic control over the cache lifecycle, making it unsuitable for the deterministic, highly repetitive loops required by the Antigravity workers.  
Explicit caching, conversely, is a manual protocol that guarantees structural control and substantial token discounts. By utilizing the API to create a CachedContent resource, the system precomputes the input tokens and generates a unique reference identifier. Subsequent queries to the model invoke this resource name, resulting in steep discounts on the referenced input tokens—specifically, a 90% discount on Gemini 2.5 models and a 75% discount on Gemini 2.0 models.

| Caching Parameter | Implicit Caching | Explicit Caching |
| :---- | :---- | :---- |
| Activation Mechanism | Automatic based on prompt prefix similarity | Programmatic API declaration |
| Cost Reduction Guarantee | Variable and non-guaranteed | Guaranteed 90% (Gemini 2.5) or 75% (Gemini 2.0) discount |
| Minimum Token Threshold | 1,024 (Flash models) / 4,096 (Pro models) | 1,024 (Flash models) / 4,096 (Pro models) |
| Maximum File Size (Direct Upload) | N/A | 10 MB (Larger payloads require Cloud Storage URIs) |
| Lifecycle Control (TTL) | Managed opaquely by the provider backend | Developer-configured (Default 1 hour, explicitly updatable) |
| Security Integrations | Standard encryption | Supports Customer-Managed Encryption Keys (CMEK) and VPC Service Controls |

### **Caching Economics and Lifecycle Management**

The financial efficacy of explicit caching in the Antigravity architecture can be modeled mathematically. Let ![][image1] represent the token count of the repository context, ![][image2] represent the average unique tokens per agent iteration, and ![][image3] represent the total number of tool-calling iterations. Without caching, the total input token cost ![][image4] is proportional to ![][image5]. With explicit caching, the cost ![][image6] is proportional to ![][image7], assuming the 90% discount rate for referenced tokens. For applications like continuous code analysis and agentic debugging over deep conversation histories, this caching structure reduces aggregate token costs by 50% to 90%, depending on the specific ratio of context size to query size.  
However, the implementation of explicit caches necessitates rigorous state management. Since cached resources incur temporal storage costs based on their Time-to-Live (TTL), an unmanaged cache will result in runaway billing. The minimum expiration time for a cache is 1 minute, while the default TTL is 60 minutes. The Orchestrator must programmatically invoke the delete method on the CachedContent endpoint immediately upon the conclusion of Phase 3 to halt billing, or utilize the patch method to extend the TTL if the worker agent requires additional iterations due to Critic rejection. Furthermore, security mandates dictate that caches containing proprietary codebases should leverage Virtual Private Cloud (VPC) Service Controls to prevent exfiltration beyond the enterprise service perimeter, optionally encrypting the cached contents using Customer-Managed Encryption Keys (CMEK).

## **The Orchestrator Lifecycle: A Four-Phase Architecture**

The Antigravity Orchestrator operates as a deterministic finite state machine, routing the execution flow through four distinct phases. This structured sequence effectively emulates a highly compressed software development methodology, described in the industry as executing a "waterfall in 15 minutes".

### **Phase 1: High-Tier Planning and Strategic Alignment**

In the initial phase, the user submits a natural language request detailing a desired feature, bug fix, or refactor. The Orchestrator provisions a high-tier reasoning model (e.g., Gemini 2.5 Pro or Gemini 3 Pro Preview) to process this request. High-tier models are explicitly engineered for complex reasoning, capable of synthesizing vast amounts of contextual information and performing sophisticated cross-modal logic.  
The Orchestrator reads the global ARCHITECTURE.md file and feeds it, alongside the user request, to the high-tier model. The system prompt instructs the model to act as a system architect, outputting a highly granular, step-by-step sequence of operations that leads to the desired final state. The resulting output is written to PLAN\_TICKET.md.  
Following the generation of the plan, the Orchestrator enforces a strict structural constraint: it suspends all background execution. This pause is the first Human-In-The-Loop (HITL) gateway. It forces the human developer and the AI to synchronize their understanding of the problem domain before any code is generated. By reviewing PLAN\_TICKET.md, the developer can identify hallucinations, scope creep, or architectural misunderstandings, editing the markdown file directly. This upfront alignment prevents the wastage of computational cycles that occurs when an LLM dives straight into code generation based on vague or misinterpreted prompts.

### **Phase 2: Autonomous Execution and Sandboxed Verification**

Upon human authorization of the PLAN\_TICKET.md, the Orchestrator initiates Phase 2\. To optimize latency and token expenditure, the system packages the entire repository state and initializes an explicit context cache. The Orchestrator then parses the approved markdown plan, extracting each distinct step.  
For each step, a narrowly scoped worker agent is spawned, powered by a low-cost, high-speed execution model (e.g., Gemini 2.5 Flash). The worker model is instantiated using the from\_cached\_content declaration, granting it instantaneous access to the deep contextual awareness of the codebase. The worker is provided with a system prompt detailing its focused objective and is granted access to a suite of structured tools.  
A critical requirement of Phase 2 is self-verification. LLMs frequently generate syntactically plausible but logically flawed code. To mitigate this, the worker agent must verify its own work before returning a diff. It achieves this via the bash\_execute tool, which provides sandboxed shell access. The worker can autonomously execute unit tests, run linters, or trigger static analysis tools, reading the standard output and standard error streams to identify failures. The agent engages in a localized reflection loop, iteratively modifying the code and re-running the tests until the output satisfies the functional requirements. Only after successful local verification does the worker formulate a final code diff using the write\_diff tool.

### **Phase 3: The Critic Agent and Alignment Assurance**

When the worker agent finalizes its task, it submits the proposed code modifications back to the Orchestrator. However, automated worker agents exhibit a known vulnerability regarding intent alignment; while their code may pass syntax checks, it may implement trivial mitigations rather than addressing the root architectural issue requested by the developer.  
To prevent misaligned code from polluting the project, the Orchestrator invokes Phase 3: Review & Critic. A separate high-tier LLM is instantiated as the Critic Agent. This agent is designed to emulate the rigorous scrutiny of a senior engineer conducting a pull request review. The Critic is supplied with the exact step from the original PLAN\_TICKET.md and the worker's submitted diff.  
The Critic operates under a highly constrained system prompt, evaluating the diff across multiple vectors: intent alignment, scope containment, execution robustness, and verification integrity. If the Critic detects a deviation from the plan, a security vulnerability, or a failure to utilize the sandboxed testing tools, it rejects the diff. It then synthesizes a detailed set of instructions and feeds them back to the worker agent, forcing a secondary execution loop. This adversarial mechanism ensures that only code meeting the precise criteria of the original plan can progress to the final stage.

### **Phase 4: Knowledge Transfer and Epistemic Updates**

If the Critic Agent approves the diff, the modifications are provisionally staged. At this juncture, the system reaches the second critical constraint: the final Human-In-The-Loop review. The developer inspects the approved diffs to ensure ultimate satisfaction.  
Upon human approval, the Orchestrator merges the code and initiates the Knowledge Transfer protocol. A high-tier agent is tasked with reading the fully executed PLAN\_TICKET.md and the old ARCHITECTURE.md. It analyzes the delta and generates an updated ARCHITECTURE.md that reflects the new state of the codebase. This might involve documenting new API endpoints, noting modified database schemas, or updating dependency lists. By immediately updating the global state document, the system prevents epistemic drift, ensuring that future agent deployments operate against the true, current state of the application rather than an outdated baseline. Finally, the Orchestrator executes a programmatic cleanup sequence, explicitly deleting the context caches to terminate billing cycles.

## **Orchestrator Logic and Python Implementation**

The following architecture defines the core logic for the Orchestrator. The implementation relies on a robust, class-based Python structure that interacts directly with generative API SDKs to manage explicit caching, explicit TTL definitions, tool execution loops, and the dual-phase HITL pauses. The implementation aggressively optimizes token passage by enforcing strict cleanup routines and utilizing context caching for all worker iterations.

Python  
import os  
import json  
import time  
from typing import List, Dict, Optional, Any  
from datetime import datetime, timezone  
\# Simulated imports based on the Google GenAI SDK and Vertex AI architecture  
from google import genai  
from google.genai import types  
from vertexai.preview import caching  
from vertexai.generative\_models import GenerativeModel, Part

class AntigravityOrchestrator:  
    """  
    Core orchestrator for the Antigravity multi-agent workflow.  
    Manages state transitions, context caching, and model routing.  
    """  
    def \_\_init\_\_(self, project\_dir: str, high\_tier\_model: str \= "gemini-2.5-pro", fast\_model: str \= "gemini-2.5-flash"):  
        self.project\_dir \= project\_dir  
        self.high\_tier\_model \= high\_tier\_model  
        self.fast\_model \= fast\_model  
        self.architecture\_path \= os.path.join(self.project\_dir, "ARCHITECTURE.md")  
        self.plan\_path \= os.path.join(self.project\_dir, "PLAN\_TICKET.md")  
        self.repo\_cache \= None

    def read\_file(self, file\_path: str) \-\> str:  
        """Utility to read the contents of a target file."""  
        if not os.path.exists(file\_path):  
            return ""  
        with open(file\_path, "r", encoding="utf-8") as f:  
            return f.read()

    def write\_file(self, file\_path: str, content: str):  
        """Utility to write persistent contents to a target file."""  
        with open(file\_path, "w", encoding="utf-8") as f:  
            f.write(content)

    def generate\_plan(self, user\_request: str):  
        """Phase 1: Planning sequence utilizing a High-Tier LLM."""  
        print("\[Phase 1\] Initializing High-Fidelity Planning Sequence...")  
        arch\_content \= self.read\_file(self.architecture\_path)  
          
        system\_prompt \= (  
            "You are an Expert AI Systems Architect. Analyze the provided architecture document "  
            "and the user request. Generate a highly granular, step-by-step execution plan. "  
            "Output the plan as a markdown file where each step represents a distinctly scoped, modular task. "  
            "Do not write any code; focus exclusively on architectural planning and task sequencing."  
        )  
          
        planning\_model \= GenerativeModel(self.high\_tier\_model, system\_instruction=system\_prompt)  
        response \= planning\_model.generate\_content()  
          
        plan\_content \= response.text  
        self.write\_file(self.plan\_path, plan\_content)  
        print(f"\[Phase 1\] Execution sequence formalized and written to {self.plan\_path}.")

    def hitl\_planning\_pause(self) \-\> bool:  
        """Constraint: Execution MUST pause for initial HITL validation."""  
        print("\\n=== HUMAN-IN-THE-LOOP REVIEW: PLANNING PHASE \===")  
        print(f"Please review {self.plan\_path}. Modify the markdown file directly to correct scope or alignment.")  
        while True:  
            user\_input \= input("Approve the plan and authorize Phase 2 execution? (yes/no): ").strip().lower()  
            if user\_input in \['yes', 'y'\]:  
                return True  
            elif user\_input in \['no', 'n'\]:  
                print("Execution aborted by human operator.")  
                return False  
            else:  
                print("Invalid input sequence. Please enter 'yes' or 'no'.")

    def initialize\_repository\_cache(self):  
        """Phase 2 Setup: Builds an explicit context cache to optimize token economics."""  
        print("\[Phase 2\] Constructing Explicit Context Cache for Repository State...")  
          
        \# In a production environment, this function aggregates all relevant source files.  
        \# For demonstration, we simulate repo ingestion using the architecture state.  
        repo\_snapshot \= self.read\_file(self.architecture\_path)   
          
        \# Explicit caching requires programmatic TTL definition.  
        self.repo\_cache \= caching.CachedContent.create(  
            model\_name=self.fast\_model,  
            system\_instruction=(  
                "You are an autonomous, narrowly scoped coding worker agent. You have access to "  
                "sandboxed bash tools. You are required to verify your logic by running automated "  
                "tests or linters before outputting final diffs."  
            ),  
            contents=\[Part.from\_text(repo\_snapshot)\],  
            ttl="3600s" \# 1-hour TTL to prevent runaway billing cycles  
        )  
        print(f"\[Phase 2\] Context Cache verified. Resource Identifier: {self.repo\_cache.name}")

    def spawn\_worker\_agent(self, task\_step: str, previous\_feedback: str \= "") \-\> str:  
        """Phase 2 Execution: Deploys a fast worker model against the cached context."""  
        \# Instantiate the model by referencing the explicitly cached tokens  
        worker\_model \= GenerativeModel.from\_cached\_content(cached\_content=self.repo\_cache)  
          
        conversation\_history \= \[  
            f"Execute the following task step based on the established plan:\\n{task\_step}"  
        \]  
        if previous\_feedback:  
            conversation\_history.append(f"Critic Agent Feedback from previous failure:\\n{previous\_feedback}")  
          
        max\_tool\_iterations \= 7  
        for iteration in range(max\_tool\_iterations):  
            \# The tool\_choice is set to "auto" allowing the model to decide between tool invocation and response  
            response \= worker\_model.generate\_content(conversation\_history)  
              
            if self.\_has\_tool\_call(response):  
                \# Simulated tool execution router  
                tool\_output \= self.\_execute\_sandboxed\_tool(response.tool\_call)  
                conversation\_history.append(f"Tool Observation Data: {tool\_output}")  
            else:  
                \# The agent has concluded tool operations and returned a diff  
                return response.text  
                  
        return "ERROR: Worker agent exhausted iteration limit without satisfying verification parameters."

    def critic\_review\_loop(self, task\_step: str, worker\_diff: str) \-\> tuple\[bool, str\]:  
        """Phase 3: High-Tier Critic evaluates the diff against the original plan ticket."""  
        print("\[Phase 3\] Invoking High-Tier Critic Agent for Alignment Verification...")  
        critic\_model \= GenerativeModel(self.high\_tier\_model)  
        critic\_system\_prompt \= self.get\_critic\_prompt()   
          
        evaluation\_payload \= (  
            f"Original Plan Step: {task\_step}\\n"  
            f"Worker Submitted Diff: {worker\_diff}\\n"  
            "Evaluate this diff against the plan. Respond strictly adhering to the mandated output format."  
        )  
          
        evaluation \= critic\_model.generate\_content(\[critic\_system\_prompt, evaluation\_payload\]).text  
          
        if "STATUS: APPROVED" in evaluation.upper():  
            print("\[Phase 3\] Code Diff Approved by Critic Agent.")  
            return True, ""  
        else:  
            print(f"\[Phase 3\] Critic Rejected Diff. Routing feedback to worker loop:\\n{evaluation}")  
            return False, evaluation

    def hitl\_merge\_pause(self, all\_diffs: List\[str\]) \-\> bool:  
        """Phase 4 Gateway: Requires human approval prior to merge and knowledge transfer."""  
        print("\\n=== HUMAN-IN-THE-LOOP REVIEW: MERGE & KNOWLEDGE TRANSFER \===")  
        print("The following diffs have been approved by the Critic Agent:")  
        for idx, diff in enumerate(all\_diffs):  
             print(f"\\n--- Diff {idx+1} \---\\n{diff}\\n-------------------")  
               
        while True:  
            user\_input \= input("Approve these changes for merge and architecture update? (yes/no): ").strip().lower()  
            if user\_input in \['yes', 'y'\]:  
                return True  
            elif user\_input in \['no', 'n'\]:  
                print("Merge aborted by human operator.")  
                return False  
            else:  
                print("Invalid input sequence. Please enter 'yes' or 'no'.")

    def execute\_workflow(self):  
        """Primary orchestration logic managing the multi-agent execution loops."""  
        plan\_content \= self.read\_file(self.plan\_path)  
        steps \=  
          
        self.initialize\_repository\_cache()  
        approved\_diffs \=  
          
        try:  
            for index, step in enumerate(steps):  
                print(f"\\n--- Executing Plan Step {index \+ 1} \---")  
                step\_approved \= False  
                attempts \= 0  
                critic\_feedback \= ""  
                  
                while not step\_approved and attempts \< 3:  
                    worker\_diff \= self.spawn\_worker\_agent(step, previous\_feedback=critic\_feedback)  
                    step\_approved, critic\_feedback \= self.critic\_review\_loop(step, worker\_diff)  
                    attempts \+= 1  
                  
                if not step\_approved:  
                    print(f"CRITICAL STATE FAILURE: Step {index \+ 1} rejected by Critic after maximum attempts. Halting.")  
                    return  
                else:  
                    approved\_diffs.append(worker\_diff)  
                      
            \# Check final HITL constraint before updating architecture  
            if self.hitl\_merge\_pause(approved\_diffs):  
                for diff in approved\_diffs:  
                    self.\_apply\_diff\_to\_codebase(diff)  
                self.update\_architecture\_state()  
            else:  
                print("Workflow terminated prior to final merge.")  
                  
        finally:  
            \# Lifecycle Cleanup: Explicit caches must be explicitly deleted to halt billing  
            if self.repo\_cache:  
                print("\\n\[Lifecycle Cleanup\] Terminating explicit repository cache resource...")  
                self.repo\_cache.delete()

    def update\_architecture\_state(self):  
        """Phase 4: Knowledge Transfer to synchronize global state."""  
        print("\[Phase 4\] Initiating Knowledge Transfer to global ARCHITECTURE.md...")  
        arch\_update\_model \= GenerativeModel(self.high\_tier\_model)  
        new\_arch \= arch\_update\_model.generate\_content(\[  
            "Update the following architecture document based on the recently executed plan.",  
            "Ensure that any new design patterns, changed dependencies, or updated API signatures are logged.",  
            f"Old Architecture: {self.read\_file(self.architecture\_path)}",  
            f"Executed Plan: {self.read\_file(self.plan\_path)}"  
        \]).text  
        self.write\_file(self.architecture\_path, new\_arch)  
        print("\[Phase 4\] Global system state synchronized. Execution complete.")

    \# \--- Abstracted utility methods for external tooling \---  
    def \_has\_tool\_call(self, response): return False  
    def \_execute\_sandboxed\_tool(self, tool\_call): return "Mock standard output: Tests passed successfully."  
    def \_apply\_diff\_to\_codebase(self, diff): pass  
    def get\_critic\_prompt(self): return "STRICT CRITIC PROMPT DEFINITION"

if \_\_name\_\_ \== "\_\_main\_\_":  
    orchestrator \= AntigravityOrchestrator(project\_dir="/opt/antigravity/workspace")  
    orchestrator.generate\_plan(user\_request="Refactor the authentication middleware to support asynchronous database connections.")  
    if orchestrator.hitl\_planning\_pause():  
        orchestrator.execute\_workflow()

This Orchestrator implementation meticulously enforces the architectural constraints. The explicit cache is instantiated immediately prior to the heavy execution loops, minimizing active resource time. The try...finally block guarantees that the explicitly created CachedContent object is deleted to prevent ongoing billing costs, an absolute necessity given the financial mechanics of explicit caching. Furthermore, it implements the precise requirement for Phase 4: the Orchestrator pauses for human approval before executing the final ARCHITECTURE.md update and codebase merge, cementing the epistemic integrity of the environment.

## **Structuring Deterministic Tool Calling Schemas**

When delegating operational authority to autonomous agents, the precision of the function-calling schema strictly defines the reliability of the system. An LLM's invocation of a tool is not an independent action; it is an expression of intent mapped to a structured application protocol. If the request matches the description provided in the schema, the model attempts to generate a structured JSON object. Without rigorously defined, deeply descriptive schemas, the model is prone to hallucinating arguments, violating data types, or entirely ignoring the tool.  
To enforce maximal structural reliability, the Antigravity system implements a "strict mode" compliance architecture for its tool schemas. Strict mode guarantees that the model's output rigidly adheres to the format requirements of the JSON schema. In a strict mode definition, all parameters must explicitly declare their type, all properties within an object must be comprehensively listed in the required array, and the additionalProperties flag must be categorically set to false. This prevents the model from injecting arbitrary keys or hallucinated sub-structures into the payload, thereby minimizing parsing exceptions within the orchestrator pipeline.  
Furthermore, defining schemas optimally enables "schema-aware drafting," a technique where predefined tool schemas serve as faithful drafts that enable the parallel verification of constrained variables. This drastically reduces latency during tool generation by minimizing the decoding steps required for the model to predict the schema structure.

### **Schema 1: Sandboxed Execution Environment (bash\_execute)**

The worker agent requires access to a controlled execution environment to verify its operations autonomously. This involves executing unit test suites, running static analysis, or invoking linters. However, providing unfettered bash access introduces severe instability risks, including the potential for the agent to execute infinite loops that hang the orchestration process. A robust execution schema must constrain the contextual directory and enforce strict timeouts.

JSON  
{  
  "name": "bash\_execute",  
  "description": "Executes a shell command in an isolated, sandboxed environment. Use this tool strictly to verify your proposed code changes by running automated tests, linters, or type checkers. It returns the standard output (stdout), standard error (stderr), and the exit code.",  
  "strict": true,  
  "parameters": {  
    "type": "object",  
    "properties": {  
      "command": {  
        "type": "string",  
        "description": "The bash command to execute. Must be completely non-interactive. Do not chain commands excessively."  
      },  
      "cwd": {  
        "type": "string",  
        "description": "The current working directory from which to execute the command. Must be provided as an absolute path or a relative path measured from the root of the repository."  
      },  
      "timeout": {  
        "type": "integer",  
        "description": "The maximum execution time permitted in seconds. This prevents hanging processes. Generally, keep this under 60 seconds unless you are invoking extensive test suites.",  
        "minimum": 1,  
        "maximum": 300,  
        "default": 30  
      }  
    },  
    "required": \["command", "cwd", "timeout"\],  
    "additionalProperties": false  
  }  
}

This schema structurally guarantees that the agent dictates the directory scope (cwd), eliminating errors caused by executing a test runner in an incorrect subdirectory. The mathematical constraints on the timeout parameter, utilizing the integer validation keys minimum and maximum, ensure that the agent cannot accidentally authorize an indefinitely running process. By setting strict to true and fully populating the required array, the Orchestrator avoids KeyError exceptions when parsing the agent's intent.

### **Schema 2: Target File Ingestion (read\_file)**

While context caching manages the bulk of the repository state at the macro level, a worker agent frequently requires precise, line-level context, or the ability to dynamically inspect files generated during its bash execution (e.g., test coverage reports, transpiled artifacts). The read\_file tool allows for targeted memory retrieval without polluting the active context window with unrelated data.

JSON  
{  
  "name": "read\_file",  
  "description": "Reads the exact contents of a specified file. Can return the entire file or a highly specific subset of lines if start\_line and end\_line parameters are intelligently defined.",  
  "strict": true,  
  "parameters": {  
    "type": "object",  
    "properties": {  
      "file\_path": {  
        "type": "string",  
        "description": "The precise absolute or relative path to the target file to be ingested."  
      },  
      "start\_line": {  
        "type": "integer",  
        "description": "The 1-indexed line number where reading should commence. If reading the entire file is absolutely necessary, set this to 1.",  
        "minimum": 1  
      },  
      "end\_line": {  
        "type": "integer",  
        "description": "The 1-indexed line number where reading should terminate. To read from the start\_line until the exact end of the file, set this to \-1."  
      }  
    },  
    "required": \["file\_path", "start\_line", "end\_line"\],  
    "additionalProperties": false  
  }  
}

By forcing the definition of start\_line and end\_line via the strict mode requirements, the schema explicitly forces the model to mathematically reason about the scope of the file it wishes to ingest. This constraint acts as an architectural guardrail, preventing the accidental ingestion of massive log files or minified javascript assets that would overwhelm the local query token threshold and disrupt the agent's cognitive flow.

### **Schema 3: Semantic Patch Generation (write\_diff)**

Modifying code via an LLM presents complex formatting and token economics challenges. Requesting a model to rewrite entire files simply to change a single variable consumes vast amounts of tokens, increases latency, and risks the introduction of unintended errors in unmodified sections. Consequently, modern autonomous coding environments utilize targeted semantic search-and-replace blocks.  
This methodology is highly robust: it requires the LLM to identify an exact, literal block of existing text (the search block) and provide its replacement (the replace block). However, this requires dealing with severe formatting idiosyncrasies. The schema must enforce literal content representation, preventing the model from utilizing markdown summaries or abbreviating the surrounding context.

JSON  
{  
  "name": "write\_diff",  
  "description": "Proposes a code change using a semantic search-and-replace block methodology. The search block must perfectly match the existing code in the file, including all whitespace, indentation, escaping, and trailing spaces. The replace block contains the newly generated code.",  
  "strict": true,  
  "parameters": {  
    "type": "object",  
    "properties": {  
      "file\_path": {  
        "type": "string",  
        "description": "The relative path to the specific file undergoing modification."  
      },  
      "changes": {  
        "type": "array",  
        "description": "An array containing discrete, proposed code changes. Each item represents a single search and replace operation within the specified file.",  
        "items": {  
          "type": "object",  
          "properties": {  
            "search\_block": {  
              "type": "string",  
              "description": "The exact, literal lines of code to be replaced. You MUST include sufficient surrounding context lines (before and after the target lines) to ensure the match is uniquely identifiable within the entire file. Do not omit any characters, containers, or whitespace."  
            },  
            "replace\_block": {  
              "type": "string",  
              "description": "The exact, literal lines of code that will seamlessly replace the search\_block."  
            }  
          },  
          "required": \["search\_block", "replace\_block"\],  
          "additionalProperties": false  
        }  
      }  
    },  
    "required": \["file\_path", "changes"\],  
    "additionalProperties": false  
  }  
}

The use of a nested array structure within the write\_diff schema enables parallel tool calling. The model can request multiple independent modifications to a single file within one structured tool call, accelerating the execution velocity. The explicit instructional density regarding whitespace, literal content, and unique matching in the property descriptions acts as an embedded heuristic. This mitigates a primary failure mode of generative models: the tendency to summarize code segments within edit blocks, which breaks deterministic regex parsers and halts integration.

## **Phase 3: The Critic Agent and Verification Logic**

Deploying an automated Critic Agent utilizing an LLM-as-a-Judge methodology inside an autonomous loop requires rigorous prompt engineering. Without strict structural boundaries, Critic Agents are prone to approving defective or misaligned code, or rejecting code based on hallucinatory standards. Studies into automated code review demonstrate a severe alignment gap; an LLM evaluator may correctly identify a minor syntax issue, or propose a trivial mitigation, while entirely missing the human reviewer's primary architectural intent.  
When a low-tier worker submits a code diff, the high-tier Critic must evaluate it not merely for syntactic correctness, but for absolute congruence with the PLAN\_TICKET.md. In raw human PR discussions, conversational noise and ambiguous requirements degrade automated review quality. Therefore, the Critic's prompt must establish an algorithmic, verifiable checklist distilled from community-agreed best practices and open-source review methodologies. The evaluation must categorically address intent alignment, scope containment, execution robustness, and verification integrity.

### **The System Prompt for Alignment Verification**

The prompt architecture provided below is designed to be injected as the system instruction for the high-tier Critic Agent. It employs strict structural constraints, demanding a deterministic output format that the Python Orchestrator can parse reliably via string matching.

# **SYSTEM ROLE**

You are the Lead Quality Assurance and Architecture Critic Agent operating within the Antigravity multi-agent system. Your singular directive is to rigorously evaluate code diffs submitted by automated Worker Agents against predetermined architectural plans.

# **EVALUATION CONTEXT**

You will be provided with the following data points:

1. The Original Plan Step extracted from PLAN\_TICKET.md. This represents the absolute ground truth for human intent and architectural requirements.  
2. The Worker Submitted Diff, representing the exact semantic code alterations proposed by the agent.  
3. The historical context of executed bash commands (tests, linters), demonstrating the worker's attempt at verification.

# **CRITICAL EVALUATION MANDATES**

You must evaluate the submission sequentially against the following criteria:

1. Intent Alignment: Does the submitted diff execute exactly what the Original Plan Step requested?  
   * Example Failure: If the plan requested an asynchronous connection protocol, and the worker implemented a synchronous wrapper, this is a catastrophic alignment failure.  
2. Scope Containment: Did the worker modify elements outside the purview of the assigned step?  
   * Example Failure: Extraneous formatting changes, unsolicited refactoring of adjacent functions, or unauthorized feature creep must be rejected immediately.  
3. Execution Robustness: Does the code contain unhandled exceptions, sub-optimal memory access patterns, or obvious security flaws (e.g., exposed secrets, injection vulnerabilities)?  
4. Verification Integrity: Did the worker adequately utilize its sandboxed bash tools to verify the code prior to submission?  
   * Example Failure: If the worker submitted a diff without executing a relevant test suite or linter where contextually appropriate, flag it as a severe process violation.

# **FEEDBACK LOOP INSTRUCTIONS**

If the code fails any of the above mandates, you must output a detailed, actionable feedback report directed at the worker. You must instruct it precisely on how to correct the failure, including specific lines to modify or bash commands to run.  
If, and only if, the code satisfies all mandates flawlessly, you must approve it.

# **OUTPUT FORMAT CONSTRAINTS**

Your output MUST adhere strictly to one of the following formats. Do not deviate, prepend, or append additional text.  
For Rejection:  
STATUS: REJECTED  
VIOLATION\_CATEGORY:  
FEEDBACK: \[Provide a highly concise, direct explanation of the failure and exact technical instructions for the worker to rectify the issue in its next iteration.\]  
For Approval:  
STATUS: APPROVED  
RATIONALE:  
This specific prompt architecture fundamentally mitigates the inherent sensitivity of LLM-as-a-judge approaches to prompt design and inherent randomness. By forcing the Critic model to categorize the rejection via the VIOLATION\_CATEGORY constraint, the model is compelled to engage in a structured chain-of-reasoning before it generates the feedback block. This deterministic step dramatically increases the precision and quality of the critique, providing the penalized worker agent with a highly localized, actionable vector for correction in the subsequent loop iteration.

## **Architectural Synthesis**

The overarching architecture of the Antigravity workflow establishes a highly optimized, stringently gated multi-agent ecosystem. By bridging high-tier reasoning capabilities with low-cost execution models, the system maintains maximal cognitive throughput. The integration of explicit context caching mathematically limits token inflation, rendering massive codebases economically traversable for automated loops. The strategic implementation of strict JSON tool calling schemas guarantees deterministic interactions between the generative agents and the underlying file system, eliminating hallucinatory parameters. Finally, the dual-tiered Human-in-the-Loop constraints—enforced during initial planning and final knowledge transfer—ensure that the probabilistic nature of large language models is thoroughly domesticated, transforming unpredictable text generation into a reliable, verifiable, and economically viable software engineering discipline.

## **Empirical Validation and 2025-2026 Research Findings**

Recent academic research and industry benchmarks from 2025 and 2026 provide empirical backing—and critical caveats—for the architectural decisions within the Antigravity framework.

### **The Economics and Efficacy of Markdown Memory**

The utilization of Markdown-driven memory (such as ARCHITECTURE.md or PLAN\_TICKET.md) has emerged as a dominant design pattern, successfully employed by high-valuation startups like Manus and massive open-source projects like OpenClaw. This "Memory as Documentation" approach allows for transparent, version-controllable, and zero-friction human-in-the-loop interventions.  
However, a 2026 study evaluating agents on the AGENTbench benchmark revealed critical constraints regarding automated context files. The research demonstrated that while human-curated context files improve task success rates by approximately 4%, LLM-generated context files actually degrade performance, reducing task success by an average of 3% while simultaneously increasing inference costs by over 20%. This strongly validates Antigravity's architectural constraint: the ARCHITECTURE.md and PLAN\_TICKET.md files must undergo rigorous human curation (the HITL pause) to be effective, rather than relying purely on autonomous model generation.

### **Sandbox Security and Execution Containment**

The Antigravity Phase 2 execution loop relies heavily on sandboxed tool verification. A 2025 report from Veracode indicated that 45% of AI-generated code fails security tests, emphasizing the danger of unverified execution. Furthermore, catastrophic failures in production—such as AI agents autonomously deleting production databases or escaping restricted directories—have proven that raw bash access is a severe vulnerability.  
Best practices established in 2025 and 2026 dictate that agent sandboxes must enforce strict network egress filtering (to prevent supply chain attacks like downloading malicious dependencies) and implement hard timeouts at the tool, loop, and lifetime levels.

### **Diff Formatting and Token Efficiency**

The schema design for the write\_diff tool utilizes a semantic search-and-replace block, which is standard practice but inherently fragile. 2026 research, such as the "To Diff or Not to Diff" study and the SWE-Edit framework, highlights that traditional find-and-replace formats suffer from exact string-matching errors, while whole-file rewrites incur prohibitive token costs. Emerging solutions propose "structure-aware" diff formats (like BlockDiff) and adaptive editing strategies (such as AdaEdit), which dynamically choose the most token-efficient format, successfully reducing both latency and cost by over 30% on long-code editing tasks.

### **Critic Agent Routing and Capabilities**

The Phase 3 Critic Agent aligns with current industry trends moving away from monolithic reviewing prompts. Research evaluating commercial agents on the c-CRAB (Code Review Agent Benchmark) dataset revealed that current state-of-the-art models only successfully solve around 40% of review tasks, underscoring the difficulty of automated code review. To combat this, enterprise architectures (such as Cloudflare's AI code review system) have transitioned to launching multiple, highly specialized sub-reviewers with tightly scoped prompts (e.g., dedicated security, performance, or compliance reviewers) rather than a single generalist judge. Furthermore, routing frameworks like SupervisorAgent have demonstrated that introducing a lightweight supervision layer can reduce token consumption by nearly 30% without compromising the success rate of the workflow.

[image1]: <data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAACgAAAAaCAYAAADFTB7LAAAB1UlEQVR4Xu2WvytGURjHHyEKSUTCIov8LJkYDSYllFIGkmKjKNPrX8BkM0i9lCQTA1nEbDFhMclkMPjx/XrO6R7HVd7e1/EO91Of3vs+5956znOfc84VSUjID4rgZgZO6WPhaIav8ECiJJ7gO7yA03ANnpvYkT4WjlnY4sUeRJMZcmLFcF802WC0wkM/KFGlSp0YW2EHDjixP4f9lPJiFaIJrnjxcrgrOql/hQm8SeBKZcIEvIUNXjwvqIHXcMEfEB07E13xBd5YMHrgM+z3BwxV8MoPhoRbDhcIqxUHJ3DvB0PBbWRPNMGfXiH7ky0QHO53Y6Kr9wUWfh3+xL5enjTcck5FK87JHMNKc70hulVZxk18GN7AOrgk0TNk3vx+wy4KVi1OVszSBe9gr2gCPPpWYZtEJ459Cy7LcA62wyYTYxF4hNbDQdhh4llhXy8nxfObxyFXOxNoNPfYCbt0i06Wq3/ExHjWz8BR0QnmBCbC6rBKfG2PolXZgmXmHq5+VrkTlsA+OAmrYdrcS1h9tw1yAvttW6KeYx/xmokzaX5MsD/ZpynR/joR/cDgfeuiEyOXoj1tWXSus4KLJ24LYowJEvcDw/63Yy5caLUSvyATEhJ+wwd6BVkN5h2vpQAAAABJRU5ErkJggg==>

[image2]: <data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAADEAAAAaCAYAAAAe97TpAAACPElEQVR4Xu2WP0hWURjGn9DCf6GSKFKDig5hkhAIoiBBQZND5uSmQ4PQUJTQ9O06VTjU1NSgEBGBoMOFlrDZRQ0qJHESBEEHy+fxPYd7PZ8N4hnuhe+BH/fc93Df+573vOcPUFFFFZ2lavL2HEzaZ/lSFzkin5AGukv+kW9kirwiX53ti32WLz0m3YFtGxbwaMZ2mXyEDShXukk+h0akGa/J2FR2H8i9jC0XUn2XAttV2CBeBvYGsgAbeO6lIP8ihxk/jybIT3I9sBdGLWSNPA07iqQ7ZJ8Mhx1FkrZbLWrNSCGlLXQRNohLQV8hpPNgHLYrHZKq090n0sBekGXXfgM7xbUBtJME6VraRFqS/rtG9z5IpskTcp8ckDGyR4bIHMyf96Ubhfz9V34hK/tnoZ3Kq5fswE5xP2s6vTtga+m3e0obpM21FbQSo8AewM6aPtIPCzSBnUEDpJPchfnxvvS/H659Yc2QLXIDFqAcP3R9JbJEat27BqiBSgpUd7FHsETo+iJlExGqhNSX+qPd196TFVIPK5VfSE9v2f0Jr6xqwCozobbKLpQSoRnL3s8k+Zc/Sb4SmI8okiOfYd10v5Nm15cgLb1bsNNeM6BBKshV1yepvJ6hPBFePnCpBzb7UW8PWvB+HWXXi6SfN7n2NZRvDq2BTWV1JfPupU1GfvSfedil05dmVP0ht0NjBGkAuu4nsIW+7p5RpZ88h+0272A7U0wp46/JLGxdjJzujq86lJdMDMlnWHoV5V7HA6Brr5K6na4AAAAASUVORK5CYII=>

[image3]: <data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABMAAAAaCAYAAABVX2cEAAABA0lEQVR4Xu2SvQ4BQRSFr4RCIaJSKyV+CpVH0IkoJB5Ao/IEngGFaCQajVIQErVolHrRqxXCOWYndmbXbqGT/ZIvkT039841IxLxKzV4gBfHkhm/WcCJpS9F2IRj+IRzGDcqRNpwDx9wCOtm7GUA1/AGC1ZGOnAEY3Zgk4crmBF1ujPMunJ+Z866ULgGT0auohq2PrFU4E5U01DYiA0JV2GzDUw639zDArFXqMK7I3+TqXyGBeI3lSvydFw5B5cwZVR8wb2ihn8+L4ENu+Id5gtX3MKyHYC+qGZH2DAjf9xPwobr6ZsNfRIJeIIzmLYyjb6MQHQRp2p7RoWCT4Mnj4j4H15INzZPvLrhKQAAAABJRU5ErkJggg==>

[image4]: <data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAEkAAAAaCAYAAAD7aXGFAAADF0lEQVR4Xu2YTahNURTHl3znO/IRhTcgEUpISQYklIS8FxMzykcxIEYKA2UgMZGJAUpmUkS6MWOkyEDqGSDJU8JEPtavtVdntbvvvnu5N6/O+dW/9tl7n7v3Xnutdfa+IhUVFYOIsaoZquFZff5cSk6r3qp+q/pU31VHVaNVs1ULiq7lAyOcVb1R7VFNSPXjVddU91SvVZNSfemYrnok5j0YK2ey6olYeym5Lbb4m9I432xTfcorywIG+qxalDdkYKRaXlkGpokZ6YZqWNaWs1K1Jq8sAz1iRtqSN1QYo1R3VI9V47K2wcZ11U+x+TLvf+Wj6pdqXd6Qw2GxlkS5EftU+/PKfriSV7SBMaoHqhN5w1+yUeyoMytvqAcLqkljI3FeuqtaljfUgTPU3ryyDXSJLWrAnW+SM9KCV5KTXool8P7gtH1JBk7swGl8VV7ZBlra+SZo2Sv5/H9T7VINDfW4+HGxg2YEz+L0vVzsNH4ktF1QDQnPXGO2pzJHDMaCQ6p5qlupD+yUwqsZ477YBvF7bCS7DzzTRh/KF8Vy6lyx310vNibz+qJakp6fSTHWezHvbBom66dpFnFZ9VD1QXUw9HO49PaKHRtwfz+hE2pPUxlYwHkpQnm1mDfg4pvEfud5aoMYAgfExpiZ2mKoLZTia4x3Y2hYmoRxGHOKaoUURj6W+kHToRbhh+aotqp2iIVN9KoIp3LueBgV+R2Pd7jbOSywNzyTq/iSOm408I+IL4Qy90X3yndS7Dx9POwwxItUhmi0yFcxzwcixL2yYxwWmzBG3CxFQt8tNmE8a3Gqx93BJ084EhYQjUYew2CE5PxUjmHMznN/xIuuii0U3NCMN1Ist75KbRE2z/Muc8crGcc3uO38UK1NZRblA2EADNEtNgk8yXcZtyeUMSQGiUYDDMIXdKrqnJgn0RfII+z8BjEvx5N4H48mNRDiJ1Nd9M4I6YR0gGeS55j3KemgkRiIXZ2YN0j9o4T34734Nwve4OEEvBvzBGXGoQ+LiZdvwsyf4zvUjQjPjs/Z+3bMOBUVFRUV/5E/AzGDX46l6goAAAAASUVORK5CYII=>

[image5]: <data:image/png;base64,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>

[image6]: <data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAADoAAAAaCAYAAADmF08eAAACi0lEQVR4Xu2XS6hOURTHl1DkncelyCPk7ZaRUgxkRnkM1J2oOzAxMiCzr24GpjISAwMxkDJwu2IgDEQpheQxICkDE7kTJff/a+3dt791Hz58Xzm3869fnbP22Wedtfbaj2NWq9ak1BSxUCwL9uliVrBVUgR4W/wUv8Rn8UCsEjPFJdGXH66qloqb4pbYKqYm+0HxTNwR38T2ZK+kesVHMWw+qqW4HzAf4adiQWtzdfTEPIgzNjrILObqB6t42RLkC7EkNhQiUBKyMTZURavNAz0RG4II9Ij5glRJHTeflztiw2TSbHFfXLXx52a3dU78EItiQ5vqF9/F19hQKgd6JdijKO/rYm5s6JDO278l+qR4GI2lpokb9vtAmb+NaOyQ2KrYsv5WOQaSNaEIYsgmXmQGxYpo7JBYxd9G4x+oR7wXh2LDWOK4x8Pbgn2TeUnEsuL+kVhsTUfYTom7Yp64YN53TurD1nQsPUep5dMVI/EmXe83fxfvXCmei8OpjVMaK3/ph2v8tH2I2S3emW8zr8Vl8co8mPXFc1mbxdF0zSF/T7J9EQeSvSwnPuixeQIQQWDLZZu3NnaAnJxr5slZl2iYl+kW84UHP22XbSkyRhmxV5JZsjqeGJE1wXZafBLL0/1La5YT7zqbrkvhj9PWLhv90YwsuwHfs8/8TwqVfnI1de20xuLFih1t96z5C0cAjDLTYad58qL4QBLC1kLi6LNXrE3XY8270g8JKv10XJRYnnuIQMg0I8LIUM6U5AbzkuOvqPxoFj6CYfRyH95JH0apYb7tlSOV+5R+Llqrn65ovvnZOP/KIa4ZHQKdYa2jjo0gShvJKvvTVrbTho9YPdkPin5q1apV6//UCOMDbvzHKMGCAAAAAElFTkSuQmCC>

[image7]: <data:image/png;base64,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>