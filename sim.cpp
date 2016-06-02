#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <set>
#include <algorithm>
#include "MersenneTwister.h"
using namespace std;

MTRand r;

class Damage {
public:
    Damage(int amount, int shield_to_avoid) :
        mAmount(amount), mShieldToAvoid(shield_to_avoid) {}
    int mAmount;
    int mShieldToAvoid;
};

class DamageCmp {
public:
    bool operator < (const Damage &d1, const Damage &d2)
    {
        if (d1.mAmount == d2.mAmount)
        {
            return d1.mShieldToAvoid < d2.mShieldToAvoid;
        }
        return d1.mAmount < d2.mAmount;
    }
};

class Ship {
public:
    virtual ~Ship() {}
    string mType;
    int mInit;
    int mIC;
    int mPC;
    int mAC;
    int mIM;
    int mPM;
    int mHull;
    int mHitPoints;
    int mComputer;
    int mShield;

    // Print blueprint abilities.
    virtual void print() const
    {
        cout << mType << " ";
        cout << "Init: " << mInit << " IC: " << mIC << " PC: " << mPC;
        cout << " AC: " << mAC << " IM: " << mIM << " PM: " << mPM;
        cout << " Hull: " << mHull;
        cout << " Comp: " << mComputer << " Shield: " << mShield << endl;
    }

    // If cannons is true, roll dice for each cannon and else
    // roll dice for each missile of this ship. For each natural 6,
    // add damage to vector 'd' with high mShieldToAvoid value. For each
    // non-natural 6 calculate the appropriate mShieldToAvoid and add the
    // damage to vector 'd'.
    void shoot(vector<Damage> &d, bool cannons) const
    {
        if (cannons)
        {
            for (int i = 0; i < mIC; i++)
            {
                shoot(d, 1);
            }
            for (int i = 0; i < mPC; i++)
            {
                shoot(d, 2);
            }
            for (int i = 0; i < mAC; i++)
            {
                shoot(d, 4);
            }
        }
        else
        {
            for (int i = 0; i < mIM; i++)
            {
                shoot(d, 1);
            }
            for (int i = 0; i < mPM; i++)
            {
                shoot(d, 2);
            }
        }
    }

    // Given dice rolls in damage vector 'd', can this ship be destroyed by
    // those the dice rolls?
    bool canBeDestroyed(const vector<Damage> &d) const
    {
        int hitpoints = mHitPoints;
        for (vector<Damage>::const_iterator it = d.begin();
             it != d.end(); ++it)
        {
            if (mShield < it->mShieldToAvoid)
            {
                hitpoints -= it->mAmount;
            }
        }
        return hitpoints <= 0;
    }

    // Given dice rolls in damage vector 'd', assign as much damage as possible
    // to this ship until there's no damage left or the ship is destroyed.
    // Remove the used dice rolls from 'd'.
    void assignDamage(vector<Damage> &d)
    {
        while (mHitPoints > 0)
        {
            vector<Damage>::iterator it;
            for (it = d.begin(); it != d.end(); ++it)
            {
                if (mShield < it->mShieldToAvoid)
                {
                    mHitPoints -= it->mAmount;
                    d.erase(it);
                    break;
                }
            }
            if (it == d.end())
            {
                // Not able to assign any more damage
                break;
            }
            if (d.empty())
            {
                break;
            }
        }
    }

private:

    // Roll dice for 'dmg' damage, incrase mShieldToAvoid by adding any
    // computer modifiers and add the dice roll to damage vector 'd', if
    // dice roll modified with computers is at least 6.
    void shoot(vector<Damage> &d, int dmg)
    {
        int roll = r.randInt(5) + 1;
        if (roll + mComputer < 6)
        {
            return;
        }
        if (roll == 6)
        {
            // should always hit
            roll += 100;
        }
        Damage hit(dmg, roll + mComputer - 5);
        d.push_back(hit);
    }
};

class Interceptor : public Ship {
};

class Cruiser : public Ship {
};

class Dreadnought : public Ship {
};

class Alien : public Ship {
};

class InitCmp {
public:
    bool operator () (const Ship *s1, const Ship *s2)
    {
        if (s1->mInit == s2->mInit)
        {
            // Need one unique element for each ship. This is an arbitrary
            // way to sort ships with same initiative.
            return s1 > s2;
        }
        return s1->mInit > s2->mInit;
    }
};

typedef vector<Ship *> ShipVector;
typedef set<Ship *, InitCmp> ShipSet;
typedef set<Ship *>::const_iterator ShipSetIt;

class Fleet {
public:
    ~Fleet()
    {
        for (ShipSetIt it = mShips.begin(); it != mShips.end(); ++it)
        {
            delete *it;
        }
    }
    void append(const vector<Ship *> &ships)
    {
        mShips.insert(ships.begin(), ships.end());
    }
    void print() const
    {
        for (ShipSetIt it = mShips.begin(); it != mShips.end(); ++it)
        {
            (*it)->print();
        }
    }
    void restore()
    {
        for (ShipSetIt it = mShips.begin(); it != mShips.end(); ++it)
        {
            (*it)->mHitPoints = (*it)->mHull;
        }
    }

    bool attack(Fleet *f)
    {
        ShipVector attacker (mShips.begin(), mShips.end());
        ShipVector defender (f->mShips.begin(), f->mShips.end());

        int init = max(attacker.front()->mInit, defender.front()->mInit);
        while (init >= 0)
        {
            shoot(defender, attacker, init, false);
            shoot(attacker, defender, init, false);
            init--;
        }

        while (!defender.empty() && !attacker.empty())
        {
            // Attacker with no cannons -> lose instantly.
            bool attacker_has_cannons = false;
            ShipVector::const_iterator it;
            for (it = attacker.begin(); it != attacker.end(); ++it)
            {
                if ((*it)->mIC || (*it)->mPC || (*it)->mAC)
                {
                    attacker_has_cannons = true;
                }
            }
            if (!attacker_has_cannons)
            {
                attacker.clear();
                break;
            }

            int init = max(attacker.front()->mInit, defender.front()->mInit);
            while (init >= 0)
            {
                shoot(defender, attacker, init, true);
                shoot(attacker, defender, init, true);
                init--;
            }
        }
        if (attacker.empty())
        {
            return false;
            cout << "Defender wins." << endl;
            for (ShipVector::const_iterator it = defender.begin();
                 it != defender.end(); ++it)
            {
                cout << (*it)->mType << endl;
            }
        }
        else
        {
            return true;
            cout << "Attacker wins." << endl;
            for (ShipVector::const_iterator it = attacker.begin();
                 it != attacker.end(); ++it)
            {
                cout << (*it)->mType << endl;
            }
        }
    }

private:

    ShipVector::iterator hasType(const string &s, ShipVector &t)
    {
        for (ShipVector::iterator it = t.begin(); it != t.end(); ++it)
        {
            if ((*it)->mType == s)
            {
                return it;
            }
        }
        return t.end();
    }

    bool assignDamage(ShipVector &target, vector<Damage> &d)
    {
        // UGH, difficult damage assigning algorithm
        while (!d.empty())
        {
            if (target.empty())
            {
                // Everything is destroyed.
                return true;
            }

            ShipVector::iterator it1 = hasType("Dreadnought", target);
            ShipVector::iterator it2 = hasType("Cruiser", target);
            if (it2 == target.end())
            {
                it2 = hasType("Alien", target);
            }
            ShipVector::iterator it3 = hasType("Interceptor", target);
            if (it1 != target.end() && (*it1)->canBeDestroyed(d))
            {
                // There is a dreadnought and it can be destroyed.
                (*it1)->assignDamage(d);
                target.erase(it1);
                continue;
            }
            else if (it2 != target.end() && (*it2)->canBeDestroyed(d))
            {
                // There was no dreadnought that could be destroyed and
                // there is a cruiser that can be destroyed.
                (*it2)->assignDamage(d);
                target.erase(it2);
                continue;
            }
            else if (it3 != target.end() && (*it3)->canBeDestroyed(d))
            {
                // There was no dreadnought or cruiser that could be destroyed
                // and there is an interceptor that can be destroyed.
                (*it3)->assignDamage(d);
                target.erase(it3);
                continue;
            }
            if (it1 != target.end())
            {
                (*it1)->assignDamage(d);
            }
            if (it2 != target.end())
            {
                (*it2)->assignDamage(d);
            }
            if (it3 != target.end())
            {
                (*it3)->assignDamage(d);
            }
            // Not able to hit anything because of shields...
            d.clear();
        }
        return false;
    }

    void shoot(ShipVector &a, ShipVector &b, int init, bool cannons)
    {
        if (a.empty() || b.empty())
        {
            return;
        }
        vector<Damage> d;
        for (int i = 0; i < a.size(); i++)
        {
            if (a[i]->mInit == init)
            {
                a[i]->shoot(d, cannons);
            }
        }
        assignDamage(b, d);
    }

    set<Ship *, InitCmp> mShips;
};

vector<Ship *> create_ships(ifstream &in)
{
    vector<Ship *> ships;
    int mod = 0;
    int count, init, ic, pc, ac, im, pm, hull, comp, shield;
    string type;
    while (in.good())
    {
        string s;
        in >> s;
        if (s == ":")
        {
            if (mod == 0)
            {
                in >> type;
                mod++;
                continue;
            }
            int i;
            in >> i;
            switch (mod)
            {
            case 1: count = i; break;
            case 2: init = i; break;
            case 3: ic = i; break;
            case 4: pc = i; break;
            case 5: ac = i; break;
            case 6: im = i; break;
            case 7: pm = i; break;
            case 8: hull = i; break;
            case 9: comp = i; break;
            case 10: shield = i; break;
            }
            mod++;
        }
        if (mod == 11)
        {
            while (count-- > 0)
            {
                Ship *ship = 0;
                if (type == "Interceptor")
                {
                    ship = new Interceptor;
                }
                else if (type == "Cruiser")
                {
                    ship = new Cruiser;
                }
                else if (type == "Dreadnought")
                {
                    ship = new Dreadnought;
                }
                else
                {
                    ship = new Alien;
                }
                ship->mType = type;
                ship->mInit = init;
                ship->mIC = ic;
                ship->mPC = pc;
                ship->mAC = ac;
                ship->mIM = im;
                ship->mPM = pm;
                ship->mHull = hull;
                ship->mHitPoints = hull;
                ship->mComputer = comp;
                ship->mShield = shield;
                ships.push_back(ship);
            }
            return ships;
        }
    }
    cout << "Malformed conf.txt" << endl;
    return ships;
}

int main()
{
    // Reading conf.txt
    Fleet *attacker = new Fleet;
    Fleet *defender = new Fleet;
    ifstream in ("conf.txt");
    if (in.is_open())
    {
        attacker->append(create_ships(in));
        attacker->append(create_ships(in));
        attacker->append(create_ships(in));
        defender->append(create_ships(in));
        in.close();
    }

    // conf.txt has now been read.

    // attacker->print();
    // defender->print();

    int N = 100000;
    int wins = 0;
    while (N-- > 0)
    {
        if (attacker->attack(defender))
        {
            wins++;
        }
        attacker->restore();
        defender->restore();
    }
    cout << "Winning %: " << (wins / 100000.0) << endl;

    delete attacker;
    delete defender;

}
