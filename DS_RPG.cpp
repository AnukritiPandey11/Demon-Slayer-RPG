#include <iostream>
#include <cstdlib>
#include <ctime>
#include <string>
#include <limits>
using namespace std;

// ===================================================
//  UTILITY: safe integer input (fixes invalid input)
// ===================================================
int getInput() {
    int val;
    while (!(cin >> val)) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Invalid input. Try again: ";
    }
    return val;
}

// ===================================================
//  ITEM
// ===================================================
struct Item {
    string name;
    int attackBonus;
    int defenseBonus;
    bool owned;

    Item(string n, int atk, int def) {
        name = n;
        attackBonus = atk;
        defenseBonus = def;
        owned = false;
    }
};

// ===================================================
//  BASE CHARACTER
// ===================================================
class Character {
public:
    int health;
    int maxHealth;

    virtual void attack() {
        cout << "Character attacks!\n";
    }
};

// ===================================================
//  PLAYER
// ===================================================
class Player : public Character {
public:
    string name;
    string charClass;   // Tanjiro / Zenitsu / Inosuke
    int score;
    int potions;        // Wisteria Potions
    int gold;           // Crow Tokens

    // Special ability tracking
    bool specialUsed;   // once per fight

    // Equipment
    Item nichirin  = Item("Nichirin Blade",   18, 0);
    Item haori     = Item("Demon Slayer Haori", 0, 8);
    Item wristguard= Item("Iron Wristguard",   0, 5);

    Player(string n, string cls) {
        name      = n;
        charClass = cls;
        score     = 0;
        potions   = 2;
        gold      = 0;
        specialUsed = false;

        // Each character has unique base stats
        if (cls == "Tanjiro") {
            health    = 110;
            maxHealth = 110;
        } else if (cls == "Zenitsu") {
            health    = 90;
            maxHealth = 90;
        } else {                 // Inosuke
            health    = 120;
            maxHealth = 120;
        }
    }

    // Base attack damage varies by character
    int getAttackDamage() {
        int base = 0;
        if (charClass == "Tanjiro")
            base = (rand() % 11) + 15;   // 15-25 balanced
        else if (charClass == "Zenitsu")
            base = (rand() % 6)  + 12;   // 12-17 weaker normal attacks
        else                              // Inosuke
            base = (rand() % 9)  + 14;   // 14-22 aggressive

        int bonus = nichirin.owned ? nichirin.attackBonus : 0;
        return base + bonus;
    }

    int getDefense() {
        int def = 0;
        if (haori.owned)      def += haori.defenseBonus;
        if (wristguard.owned) def += wristguard.defenseBonus;
        return def;
    }

    // ---- Special Abilities (once per fight) ----
    // Tanjiro: Water Breathing — deals guaranteed heavy damage
    // Zenitsu: Thunderclap Flash — massive hit but stuns self next turn
    // Inosuke: Beast Breathing — attacks twice
    int useSpecial(bool &stunned) {
        stunned = false;
        if (charClass == "Tanjiro") {
            cout << "*** WATER BREATHING: Tenth Form — Constant Flux! ***\n";
            return 55;
        } else if (charClass == "Zenitsu") {
            cout << "*** THUNDER BREATHING: First Form — Thunderclap Flash! ***\n";
            cout << "(You overexert yourself and will be stunned next turn!)\n";
            stunned = true;
            return 70;
        } else {
            cout << "*** BEAST BREATHING: Sudden Throwing Strike! ***\n";
            int hit1 = getAttackDamage();
            int hit2 = getAttackDamage();
            cout << "First hit: " << hit1 << " | Second hit: " << hit2 << "\n";
            return hit1 + hit2;
        }
    }

    void usePotion() {
        // Fix: can't heal at full HP
        if (health >= maxHealth) {
            cout << "You are already at full health!\n";
            return;
        }
        if (potions > 0) {
            int heal = 35;
            health = min(health + heal, maxHealth);
            potions--;
            cout << "You drank a Wisteria Potion! +" << heal << " HP (HP: " << health << ")\n";
        } else {
            cout << "No Wisteria Potions left!\n";
        }
    }

    void showInventory() {
        cout << "\n--- Equipment ---\n";
        cout << (nichirin.owned   ? "[E] " : "[ ] ") << nichirin.name   << " (ATK +" << nichirin.attackBonus   << ")\n";
        cout << (haori.owned      ? "[E] " : "[ ] ") << haori.name      << " (DEF +" << haori.defenseBonus     << ")\n";
        cout << (wristguard.owned ? "[E] " : "[ ] ") << wristguard.name << " (DEF +" << wristguard.defenseBonus << ")\n";
        cout << "Wisteria Potions: " << potions << " | Crow Tokens: " << gold << "\n";
        cout << "-----------------\n";
    }

    void showStats() {
        cout << "\n[" << charClass << "] [HP: " << health << "/" << maxHealth << "]"
             << " [Score: " << score << "]"
             << " [Potions: " << potions << "]"
             << " [Tokens: " << gold << "]\n";
    }
};

// ===================================================
//  ENEMY
// ===================================================
class Enemy : public Character {
public:
    string name;
    string rank;       // Lesser / Lower Moon / Upper Moon / Demon King
    int damage;
    int goldDrop;
    int phase;

    Enemy(string n, string r, int h, int d, int g = 10) {
        name     = n;
        rank     = r;
        health   = h;
        maxHealth = h;
        damage   = d;
        goldDrop = g;
        phase    = 1;
    }

    void attack() override {
        cout << name << " attacks you!\n";
    }
};

// ===================================================
//  HELPERS
// ===================================================
bool isCriticalHit()  { return (rand() % 100) < 20; }   // 20% crit
bool isMiss()         { return (rand() % 100) < 15; }   // 15% miss
bool enemyCrits()     { return (rand() % 100) < 15; }   // 15% enemy crit

// ===================================================
//  FIGHT
// ===================================================
void fight(Player &p, Enemy e) {
    cout << "\n*** " << e.rank << ": " << e.name << " appears! ***\n";
    cout << "Demon HP: " << e.health << " | Damage: " << e.damage << "\n";

    p.specialUsed = false;
    bool playerStunned = false;   // Zenitsu stun mechanic

    while (e.health > 0 && p.health > 0) {

        // Phase 2: demon rages at half HP (only Upper Moon and above)
        if (e.phase == 1 && e.health <= e.maxHealth / 2 &&
            (e.rank == "Upper Moon" || e.rank == "Demon King")) {
            e.phase  = 2;
            e.damage = (int)(e.damage * 1.5);
            cout << "\n!!! " << e.name << " reveals their Blood Demon Art! Damage surges! !!!\n";
        }

        p.showStats();
        cout << "\nDemon HP: " << e.health << "\n";

        if (playerStunned) {
            cout << "(You are stunned and cannot act this turn!)\n";
            playerStunned = false;
        } else {
            cout << "1. Attack\n";
            if (!p.specialUsed)
                cout << "2. Breathing Technique (Special — once per fight)\n";
            else
                cout << "2. Breathing Technique [USED]\n";
            cout << "3. Wisteria Potion\n4. Retreat\nEnter choice: ";

            int choice = getInput();

            if (choice == 1) {
                if (isMiss()) {
                    cout << "The demon dodged your strike!\n";
                } else {
                    int dmg = p.getAttackDamage();
                    if (isCriticalHit()) {
                        dmg = (int)(dmg * 1.8);
                        cout << "*** Total Concentration! Critical Strike! ***\n";
                    }
                    cout << "You deal " << dmg << " damage!\n";
                    e.health -= dmg;
                }
            }
            else if (choice == 2) {
                if (p.specialUsed) {
                    cout << "You have already used your Breathing Technique this fight!\n";
                } else {
                    bool stunSelf = false;
                    int dmg = p.useSpecial(stunSelf);
                    cout << "You deal " << dmg << " damage!\n";
                    e.health -= dmg;
                    p.specialUsed = true;
                    if (stunSelf) playerStunned = true;
                }
            }
            else if (choice == 3) {
                p.usePotion();
            }
            else if (choice == 4) {
                // Fix: retreating costs HP (enemy gets a free hit)
                cout << "You try to retreat!\n";
                int retreatDmg = e.damage / 2;
                if (retreatDmg < 5) retreatDmg = 5;
                p.health -= retreatDmg;
                if (p.health < 0) p.health = 0;
                cout << "The demon slashes you as you flee! You take " << retreatDmg << " damage.\n";
                p.score -= 10;
                cout << "-10 score for retreating.\n";
                return;
            }
            else {
                cout << "Invalid choice! The demon watches you in confusion.\n";
                continue;   // Fix: invalid input doesn't count as retreat
            }
        }

        // Enemy attacks if still alive
        if (e.health > 0) {
            e.attack();
            int incoming = e.damage - p.getDefense();
            if (incoming < 1) incoming = 1;

            // Enemy can crit too
            if (enemyCrits()) {
                incoming = (int)(incoming * 1.5);
                cout << "*** The demon unleashes a powerful strike! ***\n";
            }

            p.health -= incoming;
            if (p.health < 0) p.health = 0;
            cout << "You take " << incoming << " damage";
            if (p.getDefense() > 0)
                cout << " (Haori/Wristguard blocked " << p.getDefense() << ")";
            cout << "!\n";
        }
    }

    if (p.health > 0) {
        cout << "\nSunrise! " << e.name << " crumbles to ash! Well done, Demon Slayer!\n";
        p.score += 60;
        p.gold  += e.goldDrop;
        cout << "You received " << e.goldDrop << " Crow Tokens!\n";
    } else {
        cout << "\nYou have fallen... The demon stands victorious.\n";
    }
}

// ===================================================
//  SHOP (Crow Corps Supply Post)
// ===================================================
void visitShop(Player &p) {
    cout << "\n--- Crow Corps Supply Post ---\n";
    cout << "Crow Tokens: " << p.gold << "\n\n";

    bool shopping = true;
    while (shopping) {
        bool allOwned = p.nichirin.owned && p.haori.owned && p.wristguard.owned;

        cout << "What will you buy?\n";
        cout << (p.nichirin.owned   ? "1. Nichirin Blade      [Owned]\n"
                                    : "1. Nichirin Blade      (ATK +18) — 25 tokens\n");
        cout << (p.haori.owned      ? "2. Demon Slayer Haori  [Owned]\n"
                                    : "2. Demon Slayer Haori  (DEF +8)  — 20 tokens\n");
        cout << (p.wristguard.owned ? "3. Iron Wristguard     [Owned]\n"
                                    : "3. Iron Wristguard     (DEF +5)  — 15 tokens\n");
        cout << "4. Wisteria Potion (+35 HP) — 12 tokens\n";
        cout << "5. Leave\nEnter choice: ";

        int c = getInput();

        if (c == 1) {
            if (p.nichirin.owned)   cout << "You already have this!\n";
            else if (p.gold >= 25) { p.gold -= 25; p.nichirin.owned   = true; cout << "Equipped: Nichirin Blade!\n"; }
            else cout << "Not enough Crow Tokens!\n";
        }
        else if (c == 2) {
            if (p.haori.owned)      cout << "You already have this!\n";
            else if (p.gold >= 20) { p.gold -= 20; p.haori.owned      = true; cout << "Equipped: Demon Slayer Haori!\n"; }
            else cout << "Not enough Crow Tokens!\n";
        }
        else if (c == 3) {
            if (p.wristguard.owned) cout << "You already have this!\n";
            else if (p.gold >= 15) { p.gold -= 15; p.wristguard.owned = true; cout << "Equipped: Iron Wristguard!\n"; }
            else cout << "Not enough Crow Tokens!\n";
        }
        else if (c == 4) {
            if (p.gold >= 12) { p.gold -= 12; p.potions++; cout << "Bought a Wisteria Potion!\n"; }
            else cout << "Not enough Crow Tokens!\n";
        }
        else if (c == 5) {
            shopping = false;
        }
        else {
            cout << "Invalid option.\n";
        }

        // Fix: auto-exit if broke and nothing to buy
        if (p.gold < 12 && allOwned) {
            cout << "Nothing else to buy. Farewell, Slayer.\n";
            shopping = false;
        }
    }
}

// ===================================================
//  MAIN
// ===================================================
int main() {
    srand(time(0));

    cout << "=========================================\n";
    cout << "    DEMON SLAYER: KIMETSU NO YAIBA RPG   \n";
    cout << "=========================================\n";
    cout << "\nChoose your Demon Slayer:\n";
    cout << "1. Tanjiro Kamado  — Balanced | Water Breathing (55 dmg)\n";
    cout << "2. Zenitsu Agatsuma — Low HP  | Thunderclap Flash (70 dmg, self-stun)\n";
    cout << "3. Inosuke Hashibira — High HP | Beast Breathing (double strike)\n";
    cout << "Enter choice: ";

    int charChoice = getInput();
    string charClass = "Tanjiro";
    if (charChoice == 2)      charClass = "Zenitsu";
    else if (charChoice == 3) charClass = "Inosuke";

    cout << "\nEnter your name: ";
    string name;
    cin >> name;

    Player player(name, charClass);

    cout << "\nWelcome, " << charClass << " " << name << "!\n";
    cout << "The Demon Slayer Corps has sent you on a critical mission.\n";
    cout << "Demons have been spotted across the land. Eliminate them all.\n";
    cout << "Do not let the sun set on your failure.\n";

    // =====================
    // ACT 1 — THE MOUNTAIN PASS
    // =====================
    cout << "\n=== ACT 1: The Haunted Mountain Pass ===\n";

    Enemy lesserA("Flesh Demon",     "Lesser Demon",  50, 12, 10);
    Enemy lesserB("Shadow Stalker",  "Lesser Demon",  60, 14, 12);

    int act1Choice;
    cout << "\nYou reach a fork in the mountain path.\n";
    cout << "1. Take the forest trail (stealth approach)\n";
    cout << "2. Charge through the main road\nEnter choice: ";
    act1Choice = getInput();

    if (act1Choice == 1) {
        cout << "\nYou move carefully through the fog...\n";
        cout << "You spot a Flesh Demon before it sees you!\n";
        cout << "1. Ambush it (guaranteed first strike +15 dmg)\n";
        cout << "2. Observe and wait\nEnter choice: ";
        int ambushChoice = getInput();

        if (ambushChoice == 1) {
            cout << "You strike first! Dealing 15 bonus damage!\n";
            lesserA.health -= 15;
            fight(player, lesserA);
            if (player.health > 0) {
                cout << "You find a Demon Slayer Haori on a fallen slayer's body.\n";
                player.haori.owned = true;
                player.score += 15;
            }
        } else {
            cout << "You observe its movement patterns. (+10 score, no fight)\n";
            player.score += 10;
        }

        if (player.health > 0) {
            cout << "\nA second demon drops from the trees!\n";
            fight(player, lesserB);
        }
    }
    else {
        cout << "\nYou sprint down the road sword drawn!\n";
        player.gold += 20;
        cout << "You find a supply cache — 20 Crow Tokens!\n";
        visitShop(player);

        cout << "\nTwo demons block your path. The stronger one steps forward!\n";
        fight(player, lesserB);
    }

    if (player.health <= 0) {
        cout << "\n===== MISSION FAILED =====\n";
        cout << "Final Score: " << player.score << "\n";
        cout << "Ending: FALLEN SLAYER — Your name will be remembered.\n";
        return 0;
    }

    cout << "\nA Kasugai Crow drops a supply pouch. +1 Wisteria Potion!\n";
    player.potions++;
    player.showStats();

    // =====================
    // ACT 2 — THE VILLAGE
    // =====================
    cout << "\n=== ACT 2: The Demon-Infested Village ===\n";
    cout << "\nYou arrive at a burning village. Screams echo in the dark.\n";

    Enemy lowerMoon("Lower Moon Six",  "Lower Moon", 85, 20, 28);

    int villageChoice;
    cout << "1. Search for survivors first (find supplies)\n";
    cout << "2. Head straight for the demon\nEnter choice: ";
    villageChoice = getInput();

    if (villageChoice == 1) {
        cout << "\nYou rescue three villagers and find an Iron Wristguard!\n";
        player.wristguard.owned = true;
        player.gold += 15;
        player.score += 20;
        cout << "Crow Tokens found: 15\n";

        cout << "\nA supply post merchant emerges from hiding.\n";
        cout << "1. Visit the supply post\n2. Go fight the demon now\nEnter choice: ";
        int shopChoice = getInput();
        if (shopChoice == 1) visitShop(player);
    } else {
        cout << "\nYou dash toward the demon without hesitation.\n";
        player.score += 10;
        cout << "Your boldness impresses the Hashira watching from afar. +10 score.\n";
    }

    cout << "\nLower Moon Six reveals itself! This won't be easy.\n";
    fight(player, lowerMoon);

    if (player.health <= 0) {
        cout << "\n===== MISSION FAILED =====\n";
        cout << "Final Score: " << player.score << "\n";
        cout << "Ending: FALLEN SLAYER — Your name will be remembered.\n";
        return 0;
    }

    player.showStats();
    player.showInventory();

    // =====================
    // ACT 3 — THE DEMON TEMPLE
    // =====================
    cout << "\n=== ACT 3: The Demon Temple ===\n";

    Enemy upperMoon("Upper Moon Four", "Upper Moon", 110, 25, 40);

    int templeChoice;
    cout << "\nYou stand before an ancient demon temple.\n";
    cout << "A Hashira contacts you via Kasugai Crow:\n";
    cout << "\"There is an Upper Moon inside. Do not underestimate it.\"\n\n";
    cout << "1. Enter immediately\n";
    cout << "2. Scout the perimeter first (lose 8 HP, find clues)\nEnter choice: ";
    templeChoice = getInput();

    if (templeChoice == 1) {
        cout << "\nYou burst through the temple doors!\n";
        fight(player, upperMoon);
        if (player.health > 0) {
            cout << "\nYou find the demon's Nichirin Blade — forged for a Slayer long ago.\n";
            player.nichirin.owned = true;
        }
    } else {
        cout << "\nYou scout carefully, mapping its weak points.\n";
        player.health -= 8;
        if (player.health < 1) player.health = 1;
        cout << "You lose 8 HP from a hidden trap, but learn the demon's pattern.\n";
        cout << "(Upper Moon's first attack this fight will be halved.)\n";
        player.score += 10;
        upperMoon.damage = (int)(upperMoon.damage * 0.5); // first attack reduced
        fight(player, upperMoon);
        upperMoon.damage = 25; // reset (already fought)
        if (player.health > 0) {
            cout << "\nYou find the demon's Nichirin Blade among the rubble.\n";
            player.nichirin.owned = true;
        }
    }

    if (player.health <= 0) {
        cout << "\n===== MISSION FAILED =====\n";
        cout << "Final Score: " << player.score << "\n";
        cout << "Ending: FALLEN SLAYER — Your name will be remembered.\n";
        return 0;
    }

    player.showStats();
    player.showInventory();

    // =====================
    // ACT 4 — MUZAN
    // =====================
    cout << "\n=== ACT 4: The Final Confrontation ===\n";
    cout << "\nThe ground shakes. The sky turns blood red.\n";
    cout << "Kibutsuji Muzan steps from the shadows.\n";
    cout << "\"A Demon Slayer... How entertaining. Let me show you true despair.\"\n\n";

    Enemy muzan("Kibutsuji Muzan", "Demon King", 150, 30, 80);

    // Fix: show potion count before asking
    cout << "You have " << player.potions << " Wisteria Potion(s).\n";
    cout << "Before the battle:\n";
    cout << "1. Drink a potion now\n2. Save it for the fight\nEnter choice: ";
    int prepChoice = getInput();
    if (prepChoice == 1) player.usePotion();

    cout << "\n!!! DEMON KING: KIBUTSUJI MUZAN !!!\n";
    cout << "(Warning: He enters a rage at half HP!)\n";
    fight(player, muzan);

    // =====================
    // ENDING
    // =====================
    player.showStats();
    player.showInventory();

    cout << "\n=========================================\n";
    cout << "              MISSION COMPLETE            \n";
    cout << "=========================================\n";
    cout << "Slayer: " << charClass << " " << name << "\n";
    cout << "Final Score: " << player.score << "\n";

    if (player.health <= 0) {
        cout << "\nEnding: FALLEN SLAYER\n";
        cout << "You gave everything. The Corps will honor your sacrifice.\n";
    } else if (player.score >= 250) {
        cout << "\nEnding: PILLAR OF THE CORPS — HASHIRA RANK\n";
        cout << "Your name is etched into the walls of the Demon Slayer HQ.\n";
        cout << "The Hashira bow their heads. You are a legend.\n";
    } else if (player.score >= 170) {
        cout << "\nEnding: DEMON SLAYER — MISSION SUCCESS\n";
        cout << "Muzan is defeated. The sun rises over a peaceful land.\n";
    } else if (player.score >= 100) {
        cout << "\nEnding: SURVIVOR — BATTLE SCARRED\n";
        cout << "You made it, barely. There is still much training ahead.\n";
    } else {
        cout << "\nEnding: WANDERING SLAYER\n";
        cout << "You survived, but the path of a Demon Slayer is long.\n";
        cout << "Train harder. The demons will not wait.\n";
    }

    return 0;
}